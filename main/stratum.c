#include "stratum.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"

static const char *TAG = "STRATUM";

// Stratum client state
static stratum_state_t current_state = STRATUM_DISCONNECTED;
static stratum_config_t client_config;
static int socket_fd = -1;
static mining_job_t current_job;
static double current_difficulty = 1.0;
static SemaphoreHandle_t job_mutex = NULL;
static int message_id = 1;

// Extranonce received from pool
static char extranonce1[32] = {0};
static int extranonce2_size = 4;

/**
 * @brief Send a JSON-RPC message to the pool
 */
static esp_err_t stratum_send_message(const char *method, const char *params)
{
    if (socket_fd < 0) {
        return ESP_FAIL;
    }

    char buffer[1024];
    int len;
    
    if (params) {
        len = snprintf(buffer, sizeof(buffer),
                      "{\"id\":%d,\"method\":\"%s\",\"params\":%s}\n",
                      message_id++, method, params);
    } else {
        len = snprintf(buffer, sizeof(buffer),
                      "{\"id\":%d,\"method\":\"%s\",\"params\":[]}\n",
                      message_id++, method);
    }

    if (len < 0 || len >= sizeof(buffer)) {
        ESP_LOGE(TAG, "Message too long");
        return ESP_FAIL;
    }

    int sent = send(socket_fd, buffer, len, 0);
    if (sent < 0) {
        ESP_LOGE(TAG, "Send failed: %d", errno);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sent: %.*s", len - 1, buffer); // -1 to skip newline
    return ESP_OK;
}

/**
 * @brief Receive and parse a line from the pool
 */
static esp_err_t stratum_receive_line(char *buffer, size_t buffer_size, int timeout_ms)
{
    if (socket_fd < 0) {
        return ESP_FAIL;
    }

    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    size_t pos = 0;
    while (pos < buffer_size - 1) {
        int received = recv(socket_fd, &buffer[pos], 1, 0);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return ESP_ERR_TIMEOUT;
            }
            ESP_LOGE(TAG, "Receive failed: %d", errno);
            return ESP_FAIL;
        } else if (received == 0) {
            ESP_LOGW(TAG, "Connection closed by pool");
            return ESP_FAIL;
        }

        if (buffer[pos] == '\n') {
            buffer[pos] = '\0';
            return ESP_OK;
        }
        pos++;
    }

    ESP_LOGE(TAG, "Line too long");
    return ESP_FAIL;
}

/**
 * @brief Parse mining.notify message and update current job
 */
static esp_err_t stratum_parse_notify(cJSON *params)
{
    if (!cJSON_IsArray(params)) {
        ESP_LOGE(TAG, "Invalid notify params");
        return ESP_FAIL;
    }

    if (xSemaphoreTake(job_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return ESP_FAIL;
    }

    // Parse job parameters
    cJSON *job_id = cJSON_GetArrayItem(params, 0);
    cJSON *prevhash = cJSON_GetArrayItem(params, 1);
    cJSON *coinb1 = cJSON_GetArrayItem(params, 2);
    cJSON *coinb2 = cJSON_GetArrayItem(params, 3);
    cJSON *merkle_branch = cJSON_GetArrayItem(params, 4);
    cJSON *version = cJSON_GetArrayItem(params, 5);
    cJSON *nbits = cJSON_GetArrayItem(params, 6);
    cJSON *ntime = cJSON_GetArrayItem(params, 7);
    cJSON *clean_jobs = cJSON_GetArrayItem(params, 8);

    if (cJSON_IsString(job_id)) {
        strncpy(current_job.job_id, job_id->valuestring, sizeof(current_job.job_id) - 1);
    }
    if (cJSON_IsString(prevhash)) {
        strncpy(current_job.prevhash, prevhash->valuestring, sizeof(current_job.prevhash) - 1);
    }
    if (cJSON_IsString(coinb1)) {
        strncpy(current_job.coinb1, coinb1->valuestring, sizeof(current_job.coinb1) - 1);
    }
    if (cJSON_IsString(coinb2)) {
        strncpy(current_job.coinb2, coinb2->valuestring, sizeof(current_job.coinb2) - 1);
    }
    if (cJSON_IsString(version)) {
        strncpy(current_job.version, version->valuestring, sizeof(current_job.version) - 1);
    }
    if (cJSON_IsString(nbits)) {
        strncpy(current_job.nbits, nbits->valuestring, sizeof(current_job.nbits) - 1);
    }
    if (cJSON_IsString(ntime)) {
        strncpy(current_job.ntime, ntime->valuestring, sizeof(current_job.ntime) - 1);
    }
    if (cJSON_IsBool(clean_jobs)) {
        current_job.clean_jobs = cJSON_IsTrue(clean_jobs);
    }

    // Parse merkle branches
    current_job.merkle_count = 0;
    if (cJSON_IsArray(merkle_branch)) {
        int count = cJSON_GetArraySize(merkle_branch);
        for (int i = 0; i < count && i < 16; i++) {
            cJSON *branch = cJSON_GetArrayItem(merkle_branch, i);
            if (cJSON_IsString(branch)) {
                strncpy(current_job.merkle_branch[i], branch->valuestring, 
                       sizeof(current_job.merkle_branch[i]) - 1);
                current_job.merkle_count++;
            }
        }
    }

    current_job.extranonce2 = 0; // Reset extranonce2

    xSemaphoreGive(job_mutex);

    ESP_LOGI(TAG, "New job: %s (clean=%d)", current_job.job_id, current_job.clean_jobs);
    return ESP_OK;
}

/**
 * @brief Handle incoming Stratum message
 */
static void stratum_handle_message(const char *message)
{
    cJSON *json = cJSON_Parse(message);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON: %s", message);
        return;
    }

    // Check if it's a method call (notification) or a response
    cJSON *method = cJSON_GetObjectItem(json, "method");
    cJSON *result = cJSON_GetObjectItem(json, "result");
    cJSON *error_obj = cJSON_GetObjectItem(json, "error");

    if (method && cJSON_IsString(method)) {
        // Handle notifications
        cJSON *params = cJSON_GetObjectItem(json, "params");
        
        if (strcmp(method->valuestring, "mining.notify") == 0) {
            stratum_parse_notify(params);
        } else if (strcmp(method->valuestring, "mining.set_difficulty") == 0) {
            if (cJSON_IsArray(params) && cJSON_GetArraySize(params) > 0) {
                cJSON *diff = cJSON_GetArrayItem(params, 0);
                if (cJSON_IsNumber(diff)) {
                    current_difficulty = diff->valuedouble;
                    ESP_LOGI(TAG, "Difficulty set to: %.2f", current_difficulty);
                }
            }
        } else {
            ESP_LOGW(TAG, "Unknown method: %s", method->valuestring);
        }
    } else if (result || error_obj) {
        // Handle responses
        cJSON *id = cJSON_GetObjectItem(json, "id");
        
        if (error_obj && !cJSON_IsNull(error_obj)) {
            char *error_str = cJSON_Print(error_obj);
            ESP_LOGE(TAG, "Error response (id=%d): %s", 
                    id ? id->valueint : -1, error_str);
            free(error_str);
        } else if (result) {
            // Handle subscribe response
            if (current_state == STRATUM_CONNECTED) {
                if (cJSON_IsArray(result) && cJSON_GetArraySize(result) >= 2) {
                    cJSON *extranonce1_item = cJSON_GetArrayItem(result, 1);
                    cJSON *extranonce2_size_item = cJSON_GetArrayItem(result, 2);
                    
                    if (cJSON_IsString(extranonce1_item)) {
                        strncpy(extranonce1, extranonce1_item->valuestring, 
                               sizeof(extranonce1) - 1);
                    }
                    if (cJSON_IsNumber(extranonce2_size_item)) {
                        extranonce2_size = extranonce2_size_item->valueint;
                    }
                    
                    current_state = STRATUM_SUBSCRIBED;
                    ESP_LOGI(TAG, "Subscribed. Extranonce1: %s, size: %d", 
                            extranonce1, extranonce2_size);
                }
            }
            // Handle authorize response
            else if (current_state == STRATUM_SUBSCRIBED) {
                if (cJSON_IsTrue(result)) {
                    current_state = STRATUM_AUTHORIZED;
                    ESP_LOGI(TAG, "Authorized successfully");
                } else {
                    ESP_LOGE(TAG, "Authorization failed");
                    current_state = STRATUM_ERROR;
                }
            }
            // Handle share submission response
            else if (current_state == STRATUM_AUTHORIZED) {
                if (cJSON_IsTrue(result)) {
                    ESP_LOGI(TAG, "Share accepted!");
                } else {
                    ESP_LOGW(TAG, "Share rejected");
                }
            }
        }
    }

    cJSON_Delete(json);
}

/**
 * @brief Initialize Stratum client
 */
esp_err_t stratum_init(const stratum_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&client_config, config, sizeof(stratum_config_t));
    
    if (job_mutex == NULL) {
        job_mutex = xSemaphoreCreateMutex();
        if (job_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create job mutex");
            return ESP_FAIL;
        }
    }

    memset(&current_job, 0, sizeof(mining_job_t));
    current_state = STRATUM_DISCONNECTED;
    
    ESP_LOGI(TAG, "Stratum client initialized");
    return ESP_OK;
}

/**
 * @brief Connect to mining pool
 */
esp_err_t stratum_connect(void)
{
    if (current_state != STRATUM_DISCONNECTED) {
        ESP_LOGW(TAG, "Already connected or connecting");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connecting to %s:%d", client_config.pool_url, client_config.pool_port);
    current_state = STRATUM_CONNECTING;

    // Resolve hostname
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", client_config.pool_port);

    int err = getaddrinfo(client_config.pool_url, port_str, &hints, &res);
    if (err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed: %d", err);
        current_state = STRATUM_ERROR;
        return ESP_FAIL;
    }

    // Create socket
    socket_fd = socket(res->ai_family, res->ai_socktype, 0);
    if (socket_fd < 0) {
        ESP_LOGE(TAG, "Failed to create socket: %d", errno);
        freeaddrinfo(res);
        current_state = STRATUM_ERROR;
        return ESP_FAIL;
    }

    // Connect to pool
    if (connect(socket_fd, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "Connection failed: %d", errno);
        close(socket_fd);
        socket_fd = -1;
        freeaddrinfo(res);
        current_state = STRATUM_ERROR;
        return ESP_FAIL;
    }

    freeaddrinfo(res);
    current_state = STRATUM_CONNECTED;
    ESP_LOGI(TAG, "Connected to pool");

    // Send mining.subscribe
    char params[256];
    snprintf(params, sizeof(params), "[\"ESP32Miner/1.0\",null]");
    if (stratum_send_message("mining.subscribe", params) != ESP_OK) {
        stratum_disconnect();
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Disconnect from mining pool
 */
void stratum_disconnect(void)
{
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
    current_state = STRATUM_DISCONNECTED;
    ESP_LOGI(TAG, "Disconnected from pool");
}

/**
 * @brief Get current connection state
 */
stratum_state_t stratum_get_state(void)
{
    return current_state;
}

/**
 * @brief Get current mining job
 */
esp_err_t stratum_get_job(mining_job_t *job)
{
    if (!job) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(job_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_FAIL;
    }

    memcpy(job, &current_job, sizeof(mining_job_t));
    xSemaphoreGive(job_mutex);

    return ESP_OK;
}

/**
 * @brief Submit a share to the pool
 */
esp_err_t stratum_submit_share(const char *job_id, uint32_t extranonce2,
                                const char *ntime, uint32_t nonce)
{
    if (current_state != STRATUM_AUTHORIZED) {
        ESP_LOGW(TAG, "Not authorized, cannot submit share");
        return ESP_FAIL;
    }

    char params[512];
    char extranonce2_hex[32];
    char nonce_hex[16];

    // Convert to hex strings (little-endian)
    snprintf(extranonce2_hex, sizeof(extranonce2_hex), "%08x", extranonce2);
    snprintf(nonce_hex, sizeof(nonce_hex), "%08x", nonce);

    snprintf(params, sizeof(params),
            "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]",
            client_config.worker_name,
            job_id,
            extranonce2_hex,
            ntime,
            nonce_hex);

    return stratum_send_message("mining.submit", params);
}

/**
 * @brief Stratum client task
 */
void stratum_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Stratum task started on core %d", xPortGetCoreID());

    char rx_buffer[2048];
    bool authorize_sent = false;

    while (1) {
        if (current_state == STRATUM_DISCONNECTED || current_state == STRATUM_ERROR) {
            // Try to connect
            if (stratum_connect() != ESP_OK) {
                ESP_LOGW(TAG, "Connection failed, retrying in 30s...");
                vTaskDelay(pdMS_TO_TICKS(30000));
                continue;
            }
            authorize_sent = false;
        }

        if (current_state == STRATUM_SUBSCRIBED && !authorize_sent) {
            // Send mining.authorize
            char params[256];
            snprintf(params, sizeof(params), "[\"%s\",\"x\"]",
                    client_config.wallet_address);
            if (stratum_send_message("mining.authorize", params) == ESP_OK) {
                authorize_sent = true;
            }
        }

        // Receive and handle messages
        esp_err_t err = stratum_receive_line(rx_buffer, sizeof(rx_buffer), 5000);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Received: %s", rx_buffer);
            stratum_handle_message(rx_buffer);
        } else if (err == ESP_ERR_TIMEOUT) {
            // Timeout is normal, continue
            continue;
        } else {
            // Connection error
            ESP_LOGE(TAG, "Receive error, disconnecting");
            stratum_disconnect();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Get pool difficulty
 */
double stratum_get_difficulty(void)
{
    return current_difficulty;
}

/**
 * @brief Get extranonce1 from pool
 * 
 * @param buffer Buffer to store extranonce1
 * @param buffer_size Size of buffer
 * @return ESP_OK on success
 */
esp_err_t stratum_get_extranonce1(char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(buffer, extranonce1, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    
    return ESP_OK;
}

/**
 * @brief Get extranonce2 size
 * 
 * @return Extranonce2 size in bytes
 */
int stratum_get_extranonce2_size(void)
{
    return extranonce2_size;
}
