#ifndef MINING_H
#define MINING_H

#include <stdint.h>
#include "stratum.h"
#include "esp_err.h"

/**
 * @brief Construct block header from mining job
 * 
 * @param job Mining job from pool
 * @param extranonce2 Extranonce 2 value
 * @param nonce Nonce value
 * @param header Output buffer for 80-byte block header
 * @return ESP_OK on success
 */
esp_err_t mining_build_block_header(const mining_job_t *job, uint32_t extranonce2,
                                     uint32_t nonce, uint8_t *header);

/**
 * @brief Convert hex string to bytes
 * 
 * @param hex Hex string
 * @param bytes Output buffer
 * @param bytes_len Output buffer length
 * @return Number of bytes written
 */
int hex_to_bytes(const char *hex, uint8_t *bytes, size_t bytes_len);

/**
 * @brief Convert bytes to hex string
 * 
 * @param bytes Input bytes
 * @param bytes_len Length of input
 * @param hex Output hex string
 * @param hex_len Output buffer length
 * @return ESP_OK on success
 */
esp_err_t bytes_to_hex(const uint8_t *bytes, size_t bytes_len, char *hex, size_t hex_len);

/**
 * @brief Reverse byte order (for Bitcoin's little-endian format)
 * 
 * @param data Data to reverse
 * @param len Length of data
 */
void reverse_bytes(uint8_t *data, size_t len);

/**
 * @brief Calculate double SHA-256 hash
 * 
 * @param data Input data
 * @param len Input length
 * @param hash Output hash (32 bytes)
 * @return ESP_OK on success
 */
esp_err_t sha256d(const uint8_t *data, size_t len, uint8_t *hash);

/**
 * @brief Calculate merkle root from coinbase and branches
 * 
 * @param coinbase Coinbase transaction hash
 * @param branches Merkle branches
 * @param branch_count Number of branches
 * @param merkle_root Output merkle root (32 bytes)
 * @return ESP_OK on success
 */
esp_err_t mining_calculate_merkle_root(const uint8_t *coinbase, 
                                        const char branches[][65],
                                        int branch_count,
                                        uint8_t *merkle_root);

#endif // MINING_H
