#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

// Constants for I2C sweep testing
#define I2C_SWEEP_TAG     "I2C_SWEEP"
#define I2C_SWEEP_PORT    I2C_NUM_0
#define I2C_PORT          I2C_NUM_0
#define I2C_SWEEP_FREQ_HZ 100000  // 100 kHz to avoid noise during testing

static const char *TAG = "GPIO_PIN_TEST";

// ESP32-S3 I2C pin candidates for testing
static const int s3_i2c_candidates[] = {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21
};

/**
 * Initialize I2C on specific pins for testing
 */
static esp_err_t i2c_init_on_pins(int sda, int scl) {
    // Enable internal pull-ups (weak but helps reading idle state)
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
 * Deinitialize I2C
 */
static void i2c_deinit(void) {
    i2c_driver_delete(I2C_SWEEP_PORT);
}

/**
 * Scan all I2C addresses and log found devices
 * Returns the number of devices found
 */
static int i2c_scan_addrs_log(void) {
    int found = 0;
    for (uint8_t a = 1; a < 0x7F; a++) {
        i2c_cmd_handle_t c = i2c_cmd_link_create();
        i2c_master_start(c);
        i2c_master_write_byte(c, (a << 1) | I2C_MASTER_WRITE, true); // require ACK
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
 * Sweep through all possible I2C pin combinations to find working pairs
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
 * Probe a specific I2C address to check if a device responds
 * This is a drop-in replacement for i2c_master_probe on older ESP-IDF versions
 */
static esp_err_t i2c_probe_addr(i2c_port_t port, uint8_t addr, TickType_t timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * Quick probe for I2C on a specific pin pair without full sweep
 * Useful for testing a suspected SDA/SCL configuration
 */
static esp_err_t quick_probe_on_pins(int sda, int scl, uint8_t addr)
{
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
 * Try a specific SDA/SCL pair and test for common OLED addresses
 * Logs success if any device responds
 */
static esp_err_t try_pair(int sda, int scl)
{
    // Always try to delete first, ignore errors
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
        ESP_LOGW(TAG, "param_config failed SDA=%d SCL=%d: %s", sda, scl, esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "driver_install failed SDA=%d SCL=%d: %s", sda, scl, esp_err_to_name(err));
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(3));

    // Try 0x3C (standard SSD1306) and 0x3D just to be sure
    esp_err_t r1 = i2c_probe_addr(I2C_PORT, 0x3C, 50);
    esp_err_t r2 = i2c_probe_addr(I2C_PORT, 0x3D, 50);

    // Clean up before returning
    i2c_driver_delete(I2C_PORT);

    if (r1 == ESP_OK || r2 == ESP_OK) {
        ESP_LOGE(TAG,
            ">> FOUND ACK! SDA=%d SCL=%d (0x3C=%s 0x3D=%s)",
            sda, scl,
            (r1==ESP_OK?"ACK":"FAIL"),
            (r2==ESP_OK?"ACK":"FAIL"));
        return ESP_OK;
    }

    ESP_LOGI(TAG, "SDA=%d SCL=%d -> nothing found", sda, scl);
    return ESP_FAIL;
}

/**
 * Test a few plausible SDA/SCL pairs
 * Adjust the candidate arrays based on your specific board
 */
static void test_plausible_pairs(void)
{
    ESP_LOGI(TAG, "Testing plausible I2C pin pairs...");
    
    int sda_candidates[] = {8, 15, 7};      // Adjust based on your board
    int scl_candidates[] = {1, 2, 3, 4, 9}; // Adjust based on your board

    for (int si = 0; si < sizeof(sda_candidates)/sizeof(int); ++si) {
        for (int ci = 0; ci < sizeof(scl_candidates)/sizeof(int); ++ci) {

            int SDA = sda_candidates[si];
            int SCL = scl_candidates[ci];

            ESP_LOGI(TAG, "testing SDA=%d SCL=%d ...", SDA, SCL);
            esp_err_t ok = try_pair(SDA, SCL);

            // Small pause to stabilize logs
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
    
    ESP_LOGI(TAG, "Plausible pairs test complete.");
}

void app_main(void)
{
    ESP_LOGI(TAG, "GPIO Pin Test Tool Starting...");
    ESP_LOGI(TAG, "This tool helps identify I2C pin configurations on ESP32 boards");
    
    // Uncomment the test you want to run:
    
    // Option 1: Full pin sweep (tests all combinations)
    // This is comprehensive but takes time
    ESP_LOGI(TAG, "Running full I2C pin sweep...");
    i2c_pin_sweep();
    
    // Option 2: Test only plausible pairs (faster, based on your board)
    // Uncomment this and comment out i2c_pin_sweep() above if you prefer
    // test_plausible_pairs();
    
    ESP_LOGI(TAG, "All tests complete. Device will idle now.");
    
    // Keep the program running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
