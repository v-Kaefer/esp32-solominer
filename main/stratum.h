#ifndef STRATUM_H
#define STRATUM_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Stratum connection states
typedef enum {
    STRATUM_DISCONNECTED,
    STRATUM_CONNECTING,
    STRATUM_CONNECTED,
    STRATUM_SUBSCRIBED,
    STRATUM_AUTHORIZED,
    STRATUM_ERROR
} stratum_state_t;

// Mining job structure
typedef struct {
    char job_id[64];
    char prevhash[65];      // Previous block hash (32 bytes hex = 64 chars + null)
    char coinb1[256];       // Coinbase part 1
    char coinb2[256];       // Coinbase part 2
    char merkle_branch[16][65]; // Merkle branches
    int merkle_count;
    char version[9];        // Block version (4 bytes hex = 8 chars + null)
    char nbits[9];          // Difficulty bits (4 bytes hex = 8 chars + null)
    char ntime[9];          // Network time (4 bytes hex = 8 chars + null)
    bool clean_jobs;        // Should abandon previous work
    uint32_t extranonce2;   // Extranonce 2 (local counter)
} mining_job_t;

// Stratum client configuration
typedef struct {
    char pool_url[128];
    uint16_t pool_port;
    char wallet_address[64];
    char worker_name[32];
} stratum_config_t;

/**
 * @brief Initialize Stratum client
 * 
 * @param config Stratum configuration
 * @return ESP_OK on success
 */
esp_err_t stratum_init(const stratum_config_t *config);

/**
 * @brief Connect to mining pool
 * 
 * @return ESP_OK on success
 */
esp_err_t stratum_connect(void);

/**
 * @brief Disconnect from mining pool
 */
void stratum_disconnect(void);

/**
 * @brief Get current connection state
 * 
 * @return Current stratum state
 */
stratum_state_t stratum_get_state(void);

/**
 * @brief Get current mining job
 * 
 * @param job Pointer to store mining job
 * @return ESP_OK if job available
 */
esp_err_t stratum_get_job(mining_job_t *job);

/**
 * @brief Submit a share to the pool
 * 
 * @param job_id Job ID
 * @param extranonce2 Extranonce 2
 * @param ntime Network time
 * @param nonce Found nonce
 * @return ESP_OK on success
 */
esp_err_t stratum_submit_share(const char *job_id, uint32_t extranonce2, 
                                const char *ntime, uint32_t nonce);

/**
 * @brief Stratum client task (runs in background)
 * 
 * @param pvParameters Task parameters (unused)
 */
void stratum_task(void *pvParameters);

/**
 * @brief Get pool difficulty
 * 
 * @return Current mining difficulty
 */
double stratum_get_difficulty(void);

/**
 * @brief Get extranonce1 from pool
 * 
 * @param buffer Buffer to store extranonce1
 * @param buffer_size Size of buffer
 * @return ESP_OK on success
 */
esp_err_t stratum_get_extranonce1(char *buffer, size_t buffer_size);

/**
 * @brief Get extranonce2 size
 * 
 * @return Extranonce2 size in bytes
 */
int stratum_get_extranonce2_size(void);

#endif // STRATUM_H
