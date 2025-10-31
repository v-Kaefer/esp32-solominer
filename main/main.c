#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mbedtls/md.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include "driver/i2c_master.h"
#include "config.h"

// I2C Configuration for OLED
#define I2C_MASTER_SCL_IO    9    // GPIO09 na placa
#define I2C_MASTER_SDA_IO    15    // GPIO07 na placa
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000

// Bitcoin Mining Configuration
#define BTC_ADDRESS "1CW2jT4gwqyWmbAZ8HjmTLBaVg8biUiWW7"

static const char *TAG = "BTC_MINER";

// Mining statistics
static uint64_t total_hashes = 0;
static uint32_t best_difficulty = 0;
static uint32_t nonce = 0;
static uint8_t block_header[80];

// OLED device handle
static SSD1306_t dev;

// WiFi code is only compiled when WIFI_SSID is defined (i.e., when config.h exists)
// This allows CI/CD builds to succeed without WiFi credentials
#ifdef WIFI_SSID

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Retry connecting to WiFi...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// Initialize WiFi
void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init finished.");
}

#endif // WIFI_SSID

// Double SHA256 hash
void double_sha256(const uint8_t* data, size_t len, uint8_t* hash)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    
    // First SHA256
    uint8_t temp[32];
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, data, len);
    mbedtls_md_finish(&ctx, temp);
    
    // Second SHA256
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, temp, 32);
    mbedtls_md_finish(&ctx, hash);
    
    mbedtls_md_free(&ctx);
}

// Count leading zero bits in hash
uint32_t count_leading_zeros(const uint8_t* hash)
{
    uint32_t zeros = 0;
    for(int i = 31; i >= 0; i--) {
        if(hash[i] == 0) {
            zeros += 8;
        } else {
            uint8_t byte = hash[i];
            while((byte & 0x80) == 0) {
                zeros++;
                byte <<= 1;
            }
            break;
        }
    }
    return zeros;
}

// Initialize block header with mock data
void init_block_header(void)
{
    memset(block_header, 0, 80);
    
    // Version (bytes 0-3)
    uint32_t version = 0x20000000;
    memcpy(&block_header[0], &version, 4);
    
    // Previous block hash (bytes 4-35) - would be real data
    // Merkle root (bytes 36-67) - would be real data
    
    // Timestamp (bytes 68-71)
    uint32_t timestamp = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
    memcpy(&block_header[68], &timestamp, 4);
    
    // Bits/Difficulty target (bytes 72-75)
    uint32_t bits = 0x1d00ffff; // Easier target for testing
    memcpy(&block_header[72], &bits, 4);
    
    // Nonce (bytes 76-79) - will be incremented
    nonce = 0;
    memcpy(&block_header[76], &nonce, 4);
    
    ESP_LOGI(TAG, "Block header initialized");
}

// Update OLED display
void update_display(float hashrate)
{
    char line[32];
    
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    
    // Title - Page 0-1 (Yellow zone on color displays)
    ssd1306_display_text(&dev, 0, "ESP32-S3 BTC Miner", strlen("ESP32-S3 BTC Miner"), false);
    ssd1306_display_text(&dev, 1, "------------------", strlen("------------------"), false);
    
    // Hashrate - Page 2 (Blue zone on color displays)
    snprintf(line, sizeof(line), "Rate: %.1f H/s", hashrate);
    ssd1306_display_text(&dev, 2, line, strlen(line), false);
    
    // Total hashes
    snprintf(line, sizeof(line), "Total: %llu", total_hashes);
    ssd1306_display_text(&dev, 3, line, strlen(line), false);
    
    // Best difficulty
    snprintf(line, sizeof(line), "Best: %lu zeros", best_difficulty);
    ssd1306_display_text(&dev, 4, line, strlen(line), false);
    
    // Current nonce
    snprintf(line, sizeof(line), "Nonce: %lu", nonce);
    ssd1306_display_text(&dev, 5, line, strlen(line), false);
    
    // Color zone indicators - Page 6-7 (Bottom blue zone)
    ssd1306_display_text(&dev, 6, "COLOR TEST", strlen("COLOR TEST"), false);
    ssd1306_display_text(&dev, 7, "YELLOW-TOP BLUE-BTM", strlen("YELLOW-TOP BLUE-BTM"), false);
}

// Mining task
void mining_task(void *pvParameters)
{
    uint8_t hash[32];
    uint64_t hash_count = 0;
    int64_t start_time = esp_timer_get_time();
    int64_t last_update = start_time;
    
    ESP_LOGI(TAG, "Mining task started on core %d", xPortGetCoreID());
    
    init_block_header();
    
    while(1) {
        // Mine with current nonce
        double_sha256(block_header, 80, hash);
        
        hash_count++;
        total_hashes++;
        
        // Check difficulty
        uint32_t difficulty = count_leading_zeros(hash);
        
        if (difficulty > best_difficulty) {
            best_difficulty = difficulty;
            ESP_LOGI(TAG, "New best difficulty: %lu leading zeros", best_difficulty);
            
            // Print hash
            ESP_LOGI(TAG, "Hash: %02x%02x%02x%02x...%02x%02x%02x%02x",
                     hash[31], hash[30], hash[29], hash[28],
                     hash[3], hash[2], hash[1], hash[0]);
        }
        
        // Check if we found a valid block (need ~70 zeros for real Bitcoin)
        if (difficulty >= 70) {
            ESP_LOGI(TAG, "!!! BLOCK FOUND !!!");
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 2, "*** BLOCK FOUND ***", strlen("*** BLOCK FOUND ***"), false);
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
        
        // Increment nonce
        nonce++;
        memcpy(&block_header[76], &nonce, 4);
        
        // Update display every 2 seconds
        int64_t current_time = esp_timer_get_time();
        if ((current_time - last_update) >= 2000000) {
            float elapsed_sec = (current_time - start_time) / 1000000.0f;
            float hashrate = hash_count / (elapsed_sec > 0 ? elapsed_sec : 1);
            
            update_display(hashrate);
            
            last_update = current_time;
            hash_count = 0;
            start_time = current_time;
            
            ESP_LOGI(TAG, "Hashrate: %.1f H/s, Total: %llu, Best: %lu",
                     hashrate, total_hashes, best_difficulty);
        }
        
        // Small delay to prevent watchdog timeout
        if (nonce % 1000 == 0) {
            vTaskDelay(1);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 Bitcoin Miner Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize I2C using new modular driver
    ESP_LOGI(TAG, "Initializing I2C with new modular driver...");
    i2c_master_config_t i2c_config = I2C_MASTER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(i2c_master_init(&i2c_config));
    
    // Validate voltage range for display
    // Using typical ESP32 operating voltage (3.3V) - compatible with 3V OLED displays
    if (!i2c_master_validate_voltage(DISPLAY_VOLTAGE_TYPICAL_MV)) {
        ESP_LOGW(TAG, "Operating voltage outside recommended range");
    }
    
    // Detect and initialize OLED display with SSD1306/SSD1315 support
    ESP_LOGI(TAG, "Initializing OLED display...");
    display_driver_ic_t detected_driver = DISPLAY_DRIVER_SSD1306;
    esp_err_t probe_result = i2c_master_detect_driver(I2C_MASTER_NUM, 0x3C, &detected_driver);
    
    if (probe_result == ESP_OK) {
        ESP_LOGI(TAG, "Display detected: %s", i2c_master_get_driver_name(detected_driver));
        // Initialize display with detected driver IC
        i2c_master_init_ssd1306_ex(&dev, I2C_MASTER_NUM, 128, 64, 0x3C, detected_driver);
    } else {
        ESP_LOGW(TAG, "Could not detect display, using default SSD1306 initialization");
        i2c_master_init_ssd1306(&dev, I2C_MASTER_NUM, 128, 64, 0x3C);
    }

    ssd1306_contrast(&dev, 0xff);
    
    ssd1306_display_text(&dev, 0, "ESP32-S3 Miner", strlen("ESP32-S3 Miner"), false);
    ssd1306_display_text(&dev, 2, "Initializing...", strlen("Initializing..."), false);
    
#ifdef WIFI_SSID
    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi...");
    wifi_init();
    
    ssd1306_display_text(&dev, 3, "WiFi Connecting...", strlen("WiFi Connecting..."), false);
    vTaskDelay(pdMS_TO_TICKS(5000));
#endif
    
    ssd1306_display_text(&dev, 4, "Starting mining!", strlen("Starting mining!"), false);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Create mining task on Core 1 for maximum performance
    xTaskCreatePinnedToCore(
        mining_task,
        "mining_task",
        8192,
        NULL,
        5,
        NULL,
        1  // Pin to Core 1
    );
    
    ESP_LOGI(TAG, "Mining task created");
}