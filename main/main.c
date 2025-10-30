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
#include "config.h"

// I2C Configuration for OLED
#define I2C_MASTER_SCL_IO    9    // GPIO09 na placa
#define I2C_MASTER_SDA_IO    15    // GPIO07 na placa
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000
#define I2C_PORT             I2C_MASTER_NUM

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

// Initialize I2C for OLED
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


#define I2C_SWEEP_TAG     "I2C_SWEEP"
#define I2C_SWEEP_PORT    I2C_NUM_0
#define I2C_SWEEP_FREQ_HZ 100000  // 100 kHz p/ evitar ruído no teste

// Candidatos seguros no S3 (evita 0: strap, 19/20: USB D-/D+, 46: input-only)
static const int s3_i2c_candidates[] = {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21
};

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

static void i2c_deinit(void) {
    i2c_driver_delete(I2C_SWEEP_PORT);
}

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

// Drop-in p/ substituir i2c_master_probe em qualquer IDF
static esp_err_t i2c_probe_addr(i2c_port_t port, uint8_t addr, TickType_t timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true); // espera ACK
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);
    return err;
}

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

static esp_err_t try_pair(int sda, int scl)
{
    // sempre tenta deletar antes, ignora erro
    i2c_driver_delete(I2C_PORT);

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    esp_err_t err;
    err = i2c_param_config(I2C_PORT, &conf);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "param_config falhou SDA=%d SCL=%d: %s", sda, scl, esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "driver_install falhou SDA=%d SCL=%d: %s", sda, scl, esp_err_to_name(err));
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(3));

    // tenta 0x3C (SSD1306 padrão) e 0x3D só pra garantir
    esp_err_t r1 = i2c_probe_addr(I2C_PORT, 0x3C, 50);
    esp_err_t r2 = i2c_probe_addr(I2C_PORT, 0x3D, 50);

    // limpa antes de sair
    i2c_driver_delete(I2C_PORT);

    if (r1 == ESP_OK || r2 == ESP_OK) {
        ESP_LOGE(TAG,
            ">> ENCONTREI ACK! SDA=%d SCL=%d (0x3C=%s 0x3D=%s)",
            sda, scl,
            (r1==ESP_OK?"ACK":"FAIL"),
            (r2==ESP_OK?"ACK":"FAIL"));
        return ESP_OK;
    }

    ESP_LOGI(TAG, "SDA=%d SCL=%d -> nada", sda, scl);
    return ESP_FAIL;
}

static void i2c_init_fixed(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num    = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num    = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM,
                                       conf.mode,
                                       0, // rx buf disabled master
                                       0, // tx buf disabled master
                                       0));
}

void app_main(void)
{

    /* tente poucos pares plausíveis e logue somente quando der ACK
    int sda_candidates[] = {8, 15, 7};      // ajuste conforme sua placa
    int scl_candidates[] = {1, 2, 3, 4, 9};    // ajuste conforme sua placa

    for (int si = 0; si < sizeof(sda_candidates)/sizeof(int); ++si) {
        for (int ci = 0; ci < sizeof(scl_candidates)/sizeof(int); ++ci) {

            int SDA = sda_candidates[si];
            int SCL = scl_candidates[ci];

            ESP_LOGI(TAG, "testando SDA=%d SCL=%d ...", SDA, SCL);
            esp_err_t ok = try_pair(SDA, SCL);

            // pausa pequena só p/ estabilizar logs
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }*/

    // ESP_LOGI(TAG, "fim do sweep.");
    // while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }

    ESP_LOGI(TAG, "ESP32-S3 Bitcoin Miner Starting...");

    //ESP_LOGI(TAG, "I2C pin sweep (100kHz)...");
    //i2c_pin_sweep();  // <- roda uma vez para descobrir o par que responde
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize I2C
    ESP_LOGI(TAG, "Initializing I2C...");
    //ESP_ERROR_CHECK(i2c_master_init());
    // 1. Sobe I2C estável
    i2c_init_fixed();
    
    // Initialize OLED
    ESP_LOGI(TAG, "Initializing OLED...");
    i2c_master_init_ssd1306(&dev, I2C_MASTER_NUM, 128, 64, 0x3C);
    /*for (uint8_t a = 1; a < 0x7F; a++) {
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
    }*/

    //ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);
    
    ssd1306_display_text(&dev, 0, "ESP32-S3 Miner", 14, false);
    ssd1306_display_text(&dev, 2, "Initializing...", 15, false);
    
    // Initialize WiFi
    /*ESP_LOGI(TAG, "Initializing WiFi...");
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
    
    ESP_LOGI(TAG, "Mining task created");*/
}