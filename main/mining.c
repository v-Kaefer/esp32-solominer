#include "mining.h"
#include "stratum.h"
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "mbedtls/md.h"

static const char *TAG = "MINING";

/**
 * @brief Convert hex character to nibble
 */
static uint8_t hex_char_to_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

/**
 * @brief Convert hex string to bytes
 */
int hex_to_bytes(const char *hex, uint8_t *bytes, size_t bytes_len)
{
    if (!hex || !bytes) {
        return 0;
    }

    size_t hex_len = strlen(hex);
    size_t byte_count = hex_len / 2;

    if (byte_count > bytes_len) {
        byte_count = bytes_len;
    }

    for (size_t i = 0; i < byte_count; i++) {
        bytes[i] = (hex_char_to_nibble(hex[i * 2]) << 4) | 
                   hex_char_to_nibble(hex[i * 2 + 1]);
    }

    return byte_count;
}

/**
 * @brief Convert bytes to hex string
 */
esp_err_t bytes_to_hex(const uint8_t *bytes, size_t bytes_len, char *hex, size_t hex_len)
{
    if (!bytes || !hex || hex_len < (bytes_len * 2 + 1)) {
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < bytes_len; i++) {
        snprintf(&hex[i * 2], 3, "%02x", bytes[i]);
    }

    return ESP_OK;
}

/**
 * @brief Reverse byte order
 */
void reverse_bytes(uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len / 2; i++) {
        uint8_t temp = data[i];
        data[i] = data[len - 1 - i];
        data[len - 1 - i] = temp;
    }
}

/**
 * @brief Calculate double SHA-256 hash
 */
esp_err_t sha256d(const uint8_t *data, size_t len, uint8_t *hash)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    uint8_t temp[32];

    mbedtls_md_init(&ctx);
    int ret = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to setup SHA256 context: %d", ret);
        mbedtls_md_free(&ctx);
        return ESP_FAIL;
    }

    // First SHA256
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, data, len);
    mbedtls_md_finish(&ctx, temp);

    // Second SHA256
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, temp, 32);
    mbedtls_md_finish(&ctx, hash);

    mbedtls_md_free(&ctx);
    return ESP_OK;
}

/**
 * @brief Calculate merkle root from coinbase and branches
 */
esp_err_t mining_calculate_merkle_root(const uint8_t *coinbase,
                                        const char branches[][65],
                                        int branch_count,
                                        uint8_t *merkle_root)
{
    uint8_t hash[32];
    uint8_t concat[64];

    // Start with coinbase hash
    memcpy(hash, coinbase, 32);

    // Apply each merkle branch
    for (int i = 0; i < branch_count; i++) {
        uint8_t branch[32];
        hex_to_bytes(branches[i], branch, 32);

        // Concatenate hash + branch
        memcpy(concat, hash, 32);
        memcpy(concat + 32, branch, 32);

        // Double SHA-256
        if (sha256d(concat, 64, hash) != ESP_OK) {
            return ESP_FAIL;
        }
    }

    memcpy(merkle_root, hash, 32);
    return ESP_OK;
}

/**
 * @brief Construct block header from mining job
 */
esp_err_t mining_build_block_header(const mining_job_t *job, uint32_t extranonce2,
                                     uint32_t nonce, uint8_t *header)
{
    if (!job || !header) {
        return ESP_ERR_INVALID_ARG;
    }

    // Build coinbase transaction
    uint8_t coinbase_tx[512];
    size_t coinbase_len = 0;

    // Coinbase part 1
    int coinb1_len = hex_to_bytes(job->coinb1, coinbase_tx, sizeof(coinbase_tx));
    coinbase_len += coinb1_len;

    // Extranonce 1 (from pool)
    char extranonce1_str[32];
    if (stratum_get_extranonce1(extranonce1_str, sizeof(extranonce1_str)) == ESP_OK) {
        int extranonce1_len = hex_to_bytes(extranonce1_str, coinbase_tx + coinbase_len,
                                           sizeof(coinbase_tx) - coinbase_len);
        coinbase_len += extranonce1_len;
    }
    
    // Extranonce 2 (local) - convert to hex bytes
    int extranonce2_size = stratum_get_extranonce2_size();
    for (int i = 0; i < extranonce2_size && coinbase_len < sizeof(coinbase_tx); i++) {
        coinbase_tx[coinbase_len++] = (extranonce2 >> (i * 8)) & 0xFF;
    }

    // Coinbase part 2
    int coinb2_len = hex_to_bytes(job->coinb2, coinbase_tx + coinbase_len,
                                   sizeof(coinbase_tx) - coinbase_len);
    coinbase_len += coinb2_len;

    // Hash coinbase transaction
    uint8_t coinbase_hash[32];
    if (sha256d(coinbase_tx, coinbase_len, coinbase_hash) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to hash coinbase");
        return ESP_FAIL;
    }

    // Calculate merkle root
    uint8_t merkle_root[32];
    if (mining_calculate_merkle_root(coinbase_hash, job->merkle_branch,
                                     job->merkle_count, merkle_root) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calculate merkle root");
        return ESP_FAIL;
    }

    // Construct block header (80 bytes)
    memset(header, 0, 80);

    // Version (4 bytes) - little endian
    uint32_t version;
    hex_to_bytes(job->version, (uint8_t*)&version, 4);
    memcpy(header, &version, 4);

    // Previous block hash (32 bytes) - need to reverse from hex
    uint8_t prevhash[32];
    hex_to_bytes(job->prevhash, prevhash, 32);
    reverse_bytes(prevhash, 32);
    memcpy(header + 4, prevhash, 32);

    // Merkle root (32 bytes)
    memcpy(header + 36, merkle_root, 32);

    // Timestamp (4 bytes) - little endian
    uint32_t timestamp;
    hex_to_bytes(job->ntime, (uint8_t*)&timestamp, 4);
    memcpy(header + 68, &timestamp, 4);

    // Difficulty bits (4 bytes) - little endian
    uint32_t bits;
    hex_to_bytes(job->nbits, (uint8_t*)&bits, 4);
    memcpy(header + 72, &bits, 4);

    // Nonce (4 bytes) - little endian
    memcpy(header + 76, &nonce, 4);

    return ESP_OK;
}
