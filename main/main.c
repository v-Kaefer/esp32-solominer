/**
 * @file main.c
 * @brief ESP32-S3 Bitcoin Solo Miner
 * 
 * This application implements a Bitcoin solo miner for the ESP32-S3 microcontroller.
 * It performs SHA256 double hashing to mine Bitcoin blocks, displays statistics on
 * an SSD1306 OLED display, and connects to WiFi for potential pool communication.
 * 
 * @note This is an educational/testing project. The ESP32's hashrate is far too low
 * for practical Bitcoin mining.
 */

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
#include "ssd1306.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "../.env"


// WiFi Configuration
#define WIFI_SSID $ENV{WIFI_SSID_VAR}  /*!< WiFi SSID from environment */
#define WIFI_PASS $ENV{WIFI_PASS_VAR}  /*!< WiFi password from environment */

// I2C Configuration for OLED
#define I2C_MASTER_SCL_IO    8    /*!< GPIO pin for I2C SCL */
#define I2C_MASTER_SDA_IO    15   /*!< GPIO pin for I2C SDA */
#define I2C_MASTER_NUM       I2C_NUM_0  /*!< I2C port number */
#define I2C_MASTER_FREQ_HZ   100000     /*!< I2C master clock frequency */

// Bitcoin Mining Configuration
#define BTC_ADDRESS "1CW2jT4gwqyWmbAZ8HjmTLBaVg8biUiWW7"  /*!< Bitcoin address for mining rewards */

static const char *TAG = "BTC_MINER";

// Mining statistics
static uint64_t total_hashes = 0;   /*!< Total number of hashes computed */
static uint32_t best_difficulty = 0; /*!< Best difficulty (leading zeros) found */
static uint32_t nonce = 0;          /*!< Current nonce value */
static uint8_t block_header[80];    /*!< Bitcoin block header (80 bytes) */

// OLED device handle
static SSD1306_t dev;

/**
 * @brief WiFi event handler
 * 
 * Handles WiFi connection events including start, disconnect, and IP assignment.
 * 
 * @param arg User argument
 * @param event_base Event base (WIFI_EVENT or IP_EVENT)
 * @param event_id Specific event ID
 * @param event_data Event data pointer
 */
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

/**
 * @brief Initialize WiFi in station mode
 * 
 * Sets up WiFi connection with credentials from environment variables.
 * Registers event handlers for WiFi and IP events.
 */
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

/**
 * @brief Initialize I2C master interface
 * 
 * Configures and initializes I2C bus for communication with OLED display.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

/**
 * @brief Compute double SHA256 hash
 * 
 * Performs two rounds of SHA256 hashing as required by Bitcoin protocol.
 * 
 * @param data Input data to hash
 * @param len Length of input data in bytes
 * @param hash Output buffer for 32-byte hash result
 */
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

/**
 * @brief Count leading zero bits in hash
 * 
 * Counts the number of leading zero bits in a hash to determine mining difficulty.
 * Used to check if a hash meets the required difficulty target.
 * 
 * @param hash 32-byte hash to analyze
 * @return Number of leading zero bits
 */
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

/**
 * @brief Initialize Bitcoin block header
 * 
 * Sets up a mock Bitcoin block header with version, timestamp, difficulty bits,
 * and initial nonce value. In a real miner, this would contain actual block data
 * from the network.
 */
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

/**
 * @brief Update OLED display with mining statistics
 * 
 * Displays current hashrate, total hashes, best difficulty found, and current nonce
 * on the OLED screen.
 * 
 * @param hashrate Current mining hashrate in hashes per second
 */
void update_display(float hashrate)
{
    char line[32];
    
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    
    // Title
    ssd1306_display_text(&dev, 0, "ESP32-S3 BTC Miner", 18, false);
    ssd1306_display_text(&dev, 1, "------------------", 18, false);
    
    // Hashrate
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
}

/**
 * @brief Main mining task
 * 
 * Continuously computes SHA256 hashes by incrementing the nonce value.
 * Tracks statistics and updates the display every 2 seconds.
 * 
 * @param pvParameters Task parameters (unused)
 */
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
            ssd1306_display_text(&dev, 2, "*** BLOCK FOUND ***", 19, false);
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


#define I2C_SWEEP_TAG     "I2C_SWEEP"  /*!< Tag for I2C sweep logging */
#define I2C_SWEEP_PORT    I2C_NUM_0    /*!< I2C port for sweep */
#define I2C_SWEEP_FREQ_HZ 100000       /*!< I2C frequency for sweep (100 kHz) */

/**
 * @brief GPIO pins safe to use on ESP32-S3
 * 
 * Excludes strapping pins (0), USB pins (19, 20), and input-only pins (46).
 */
static const int s3_i2c_candidates[] = {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21
};

/**
 * @brief Initialize I2C on specific GPIO pins
 * 
 * @param sda SDA GPIO pin number
 * @param scl SCL GPIO pin number
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t i2c_init_on_pins(int sda, int scl) {
    // puxa as linhas para alto (pull-up interno é fraco, mas ajuda na leitura de idle)
    gpio_set_pull_mode(sda, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(scl, GPIO_PULLUP_ONLY);

    i2c_config_t conf = {0};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_SWEEP_FREQ_HZ;

    esp_err_t err = i2c_param_config(I2C_SWEEP_PORT, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_SWEEP_PORT, conf.mode, 0, 0, 0);
}

/**
 * @brief Deinitialize I2C driver
 */
static void i2c_deinit(void) {
    i2c_driver_delete(I2C_SWEEP_PORT);
}

/**
 * @brief Scan all I2C addresses and log devices found
 * 
 * @return Number of I2C devices found
 */
static int i2c_scan_addrs_log(void) {
    int found = 0;
    for (uint8_t a = 1; a < 0x7F; a++) {
        i2c_cmd_handle_t c = i2c_cmd_link_create();
        i2c_master_start(c);
        i2c_master_write_byte(c, (a << 1) | I2C_MASTER_WRITE, true); // exige ACK
        i2c_master_stop(c);
        esp_err_t r = i2c_master_cmd_begin(I2C_SWEEP_PORT, c, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(c);
        if (r == ESP_OK) {
            ESP_LOGW(I2C_SWEEP_TAG, "addr 0x%02X", a);
            found++;
        }
    }
    return found;
}

/**
 * @brief Sweep through GPIO pin combinations to find I2C devices
 * 
 * Tests different SDA/SCL pin combinations to automatically detect
 * which pins are connected to I2C devices.
 */
static void i2c_pin_sweep(void) {
    ESP_LOGI(I2C_SWEEP_TAG, "=== starting sweep @100kHz ===");
    size_t n = sizeof(s3_i2c_candidates)/sizeof(s3_i2c_candidates[0]);

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            int sda = s3_i2c_candidates[i];
            int scl = s3_i2c_candidates[j];
            if (sda == scl) continue;
            if (sda == 0 || scl == 0) continue;
            if (sda == 19 || sda == 20 || scl == 19 || scl == 20) continue; // USB
            if (sda == 46 || scl == 46) continue; // input-only

            esp_err_t err = i2c_init_on_pins(sda, scl);
            if (err == ESP_OK) {
                int idle_sda = gpio_get_level(sda);
                int idle_scl = gpio_get_level(scl);
                ESP_LOGI(I2C_SWEEP_TAG, "try SDA=%d SCL=%d (idle %d/%d)",
                         sda, scl, idle_sda, idle_scl);

                int cnt = i2c_scan_addrs_log();
                if (cnt > 0) {
                    ESP_LOGW(I2C_SWEEP_TAG, "FOUND: SDA=%d SCL=%d | %d device(s)",
                             sda, scl, cnt);
                }
                i2c_deinit();
                vTaskDelay(pdMS_TO_TICKS(10));
            } else {
                ESP_LOGW(I2C_SWEEP_TAG, "skip SDA=%d SCL=%d (init err=%d)", sda, scl, err);
            }
        }
    }
    ESP_LOGI(I2C_SWEEP_TAG, "=== sweep done ===");
}

/**
 * @brief Probe a specific GPIO pin by reading its level
 * 
 * Useful for testing if a GPIO pin is properly connected by grounding it.
 * 
 * @param gpio GPIO pin number to probe
 */
static void probe_pin_is_really_here(int gpio) {
  gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);
  gpio_set_direction(gpio, GPIO_MODE_INPUT);
  for (int i=0;i<100;i++) {
    int lvl = gpio_get_level(gpio);
    ESP_LOGI("PROBE", "GPIO%d level=%d (encoste ao GND p/ ver 0)", gpio, lvl);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

/**
 * @brief Probe I2C address on specific pins
 * 
 * Compatibility function for older ESP-IDF versions without i2c_master_probe.
 * 
 * @param port I2C port number
 * @param addr I2C device address
 * @param timeout_ms Timeout in milliseconds
 * @return ESP_OK if device responds, error code otherwise
 */
static esp_err_t i2c_probe_addr(i2c_port_t port, uint8_t addr, TickType_t timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // bit R/W = 0 (WRITE). O ACK do slave confirma a presença.
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);
    return err;
}

/**
 * @brief Probe I2C device on specific pin combination
 * 
 * @param sda SDA GPIO pin
 * @param scl SCL GPIO pin  
 * @param addr I2C device address to probe
 * @return ESP_OK if device found, error code otherwise
 */
static esp_err_t probe_once(int sda, int scl, int addr) {
    i2c_driver_delete(I2C_NUM_0); // ok se já estava deletado

    i2c_config_t c = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda, .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl, .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &c));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, c.mode, 0, 0, 0));
    vTaskDelay(pdMS_TO_TICKS(3));

    esp_err_t r = i2c_probe_addr(I2C_NUM_0, addr, 50);
    i2c_driver_delete(I2C_NUM_0);
    return r;
}



/**
 * @brief Main application entry point
 * 
 * Initializes all subsystems (NVS, I2C, OLED, WiFi) and starts the mining task.
 * Includes diagnostic I2C pin sweep to help identify connected devices.
 */
void app_main(void)
{

    // tente poucos pares plausíveis e logue somente quando der ACK
    int sda_candidates[] = {8, 15, 7};      // ajuste conforme sua placa
    int scl_candidates[] = {1, 2, 3, 4, 9};    // ajuste conforme sua placa

    for (int si = 0; si < sizeof(sda_candidates)/4; ++si) {
        for (int ci = 0; ci < sizeof(scl_candidates)/4; ++ci) {
            int SDA = sda_candidates[si], SCL = scl_candidates[ci];
            if (probe_once(SDA, SCL, 0x3C) == ESP_OK ||
                probe_once(SDA, SCL, 0x3D) == ESP_OK) {
                ESP_LOGW("I2C", "ACK em 0x3C/0x3D com SDA=%d SCL=%d", SDA, SCL);
            }
        }
    }

    ESP_LOGI(TAG, "ESP32-S3 Bitcoin Miner Starting...");
    //probe_pin_is_really_here(16); // teste de pino com GND fisico
    //probe_pin_is_really_here(15); // teste de pino com GND fisico

    ESP_LOGI(TAG, "I2C pin sweep (100kHz)...");
    i2c_pin_sweep();  // <- roda uma vez para descobrir o par que responde

    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize I2C
    ESP_LOGI(TAG, "Initializing I2C...");
    ESP_ERROR_CHECK(i2c_master_init());
    
    // Initialize OLED
    ESP_LOGI(TAG, "Initializing OLED...");
    for (uint8_t a = 1; a < 0x7F; a++) {
        i2c_cmd_handle_t c = i2c_cmd_link_create();
        i2c_master_start(c);
        i2c_master_write_byte(c, (a << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(c);
        esp_err_t r = i2c_master_cmd_begin(I2C_MASTER_NUM, c, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(c);
        if (r == ESP_OK) {
            ESP_LOGI("I2C", "ACHOU em 0x%02X", a);
        } else {
            // descomente se quiser ver o motivo
            ESP_LOGW("I2C", "0x%02X -> %s", a, esp_err_to_name(r));
        }
    }

    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    
    ssd1306_display_text(&dev, 0, "ESP32-S3 Miner", 14, false);
    ssd1306_display_text(&dev, 2, "Initializing...", 15, false);
    
    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi...");
    wifi_init();
    
    ssd1306_display_text(&dev, 3, "WiFi Connecting...", 18, false);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    ssd1306_display_text(&dev, 4, "Starting mining!", 16, false);
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