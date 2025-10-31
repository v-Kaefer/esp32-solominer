#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <stdint.h>
#include "esp_err.h"

// MQTT Configuration - default values, can be overridden in config.h
#ifndef MQTT_BROKER_URL
#define MQTT_BROKER_URL "mqtt://broker.emqx.io:1883"
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "esp32_btc_miner"
#endif

// MQTT Topics
#define MQTT_TOPIC_HASHRATE "btc_miner/hashrate"
#define MQTT_TOPIC_TOTAL_HASHES "btc_miner/total_hashes"
#define MQTT_TOPIC_BEST_DIFFICULTY "btc_miner/best_difficulty"
#define MQTT_TOPIC_STATUS "btc_miner/status"

/**
 * @brief Initialize MQTT client and connect to broker
 * 
 * This function starts MQTT client on Core 0 to not disrupt SHA256 mining on Core 1
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_handler_init(void);

/**
 * @brief Publish mining statistics to MQTT broker
 * 
 * @param hashrate Current mining hashrate (hashes per second)
 * @param total_hashes Total hashes computed since start
 * @param best_difficulty Best difficulty found (leading zeros)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_publish_mining_stats(float hashrate, uint64_t total_hashes, uint32_t best_difficulty);

/**
 * @brief Publish status message to MQTT broker
 * 
 * @param status Status message string
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_publish_status(const char *status);

/**
 * @brief Check if MQTT client is connected
 * 
 * @return true if connected, false otherwise
 */
bool mqtt_is_connected(void);

/**
 * @brief Stop MQTT client
 */
void mqtt_handler_stop(void);

#endif // MQTT_HANDLER_H
