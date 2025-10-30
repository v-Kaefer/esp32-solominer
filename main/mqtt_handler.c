#include "mqtt_handler.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT_HANDLER";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

// MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_connected = true;
            mqtt_publish_status("ESP32 Bitcoin Miner Online");
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_connected = false;
            break;
            
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGE(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGE(TAG, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno,
                         strerror(event->error_handle->esp_transport_sock_errno));
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGE(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;
            
        default:
            ESP_LOGD(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

esp_err_t mqtt_handler_init(void)
{
    if (mqtt_client != NULL) {
        ESP_LOGW(TAG, "MQTT client already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing MQTT client on Core %d", xPortGetCoreID());
    
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URL,
        .credentials.client_id = MQTT_CLIENT_ID,
        .network.timeout_ms = 5000,
        .network.refresh_connection_after_ms = 60000,
        .buffer.size = 1024,
        .buffer.out_size = 1024,
    };
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    
    esp_err_t err = esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register MQTT event handler: %s", esp_err_to_name(err));
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        return err;
    }
    
    err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        return err;
    }
    
    ESP_LOGI(TAG, "MQTT client started successfully");
    return ESP_OK;
}

esp_err_t mqtt_publish_mining_stats(float hashrate, uint64_t total_hashes, uint32_t best_difficulty)
{
    if (mqtt_client == NULL || !mqtt_connected) {
        ESP_LOGD(TAG, "MQTT not connected, skipping publish");
        return ESP_ERR_INVALID_STATE;
    }
    
    char payload[128];
    int msg_id;
    
    // Publish hashrate
    snprintf(payload, sizeof(payload), "%.2f", hashrate);
    msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_HASHRATE, payload, 0, 0, 0);
    if (msg_id < 0) {
        ESP_LOGW(TAG, "Failed to publish hashrate");
    }
    
    // Publish total hashes
    snprintf(payload, sizeof(payload), "%llu", total_hashes);
    msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_TOTAL_HASHES, payload, 0, 0, 0);
    if (msg_id < 0) {
        ESP_LOGW(TAG, "Failed to publish total hashes");
    }
    
    // Publish best difficulty
    snprintf(payload, sizeof(payload), "%lu", best_difficulty);
    msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_BEST_DIFFICULTY, payload, 0, 0, 0);
    if (msg_id < 0) {
        ESP_LOGW(TAG, "Failed to publish best difficulty");
    }
    
    ESP_LOGD(TAG, "Published mining stats - Rate: %.2f H/s, Total: %llu, Best: %lu", 
             hashrate, total_hashes, best_difficulty);
    
    return ESP_OK;
}

esp_err_t mqtt_publish_status(const char *status)
{
    if (mqtt_client == NULL) {
        ESP_LOGD(TAG, "MQTT client not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_STATUS, status, 0, 0, 0);
    if (msg_id < 0) {
        ESP_LOGW(TAG, "Failed to publish status");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Published status: %s", status);
    return ESP_OK;
}

bool mqtt_is_connected(void)
{
    return mqtt_connected;
}

void mqtt_handler_stop(void)
{
    if (mqtt_client != NULL) {
        mqtt_publish_status("ESP32 Bitcoin Miner Offline");
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        mqtt_connected = false;
        ESP_LOGI(TAG, "MQTT client stopped");
    }
}
