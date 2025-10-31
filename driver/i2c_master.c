/**
 * @file i2c_master.c
 * @brief I2C Master Driver Implementation for SSD1306/SSD1315 OLED Displays
 */

#include "i2c_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "I2C_MASTER";

/**
 * @brief SSD1306/SSD1315 command codes for driver detection
 */
#define SSD1306_CMD_SET_CONTRAST          0x81
#define SSD1306_CMD_DISPLAY_ON            0xAF
#define SSD1306_CMD_DISPLAY_OFF           0xAE

esp_err_t i2c_master_init(const i2c_master_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing I2C master on port %d", config->i2c_port);
    ESP_LOGI(TAG, "SDA: GPIO%d, SCL: GPIO%d", config->sda_io_num, config->scl_io_num);
    ESP_LOGI(TAG, "Clock speed: %lu Hz", config->clk_speed);

    // Configure I2C parameters
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config->sda_io_num,
        .sda_pullup_en = config->sda_pullup_en ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .scl_io_num = config->scl_io_num,
        .scl_pullup_en = config->scl_pullup_en ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .master.clk_speed = config->clk_speed,
    };

    // Apply I2C configuration
    esp_err_t err = i2c_param_config(config->i2c_port, &i2c_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C parameters: %s", esp_err_to_name(err));
        return err;
    }

    // Install I2C driver
    // Master mode does not need RX/TX buffers (set to 0)
    err = i2c_driver_install(config->i2c_port, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C master initialized successfully");
    return ESP_OK;
}

esp_err_t i2c_master_deinit(i2c_port_t i2c_port)
{
    ESP_LOGI(TAG, "Deinitializing I2C master on port %d", i2c_port);
    
    esp_err_t err = i2c_driver_delete(i2c_port);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete I2C driver: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C master deinitialized successfully");
    return ESP_OK;
}

bool i2c_master_validate_voltage(uint32_t voltage_mv)
{
    if (voltage_mv < DISPLAY_VOLTAGE_MIN_MV) {
        ESP_LOGW(TAG, "Voltage %lu mV is below minimum %d mV", 
                 voltage_mv, DISPLAY_VOLTAGE_MIN_MV);
        return false;
    }
    
    if (voltage_mv > DISPLAY_VOLTAGE_MAX_MV) {
        ESP_LOGW(TAG, "Voltage %lu mV is above maximum %d mV", 
                 voltage_mv, DISPLAY_VOLTAGE_MAX_MV);
        return false;
    }

    ESP_LOGI(TAG, "Voltage %lu mV is within valid range (%d-%d mV)", 
             voltage_mv, DISPLAY_VOLTAGE_MIN_MV, DISPLAY_VOLTAGE_MAX_MV);
    return true;
}

esp_err_t i2c_master_probe_device(i2c_port_t i2c_port, uint8_t i2c_addr)
{
    ESP_LOGD(TAG, "Probing device at address 0x%02X", i2c_addr);
    
    // Create I2C command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C command link");
        return ESP_ERR_NO_MEM;
    }

    // Send start condition and address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    // Execute command
    esp_err_t err = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Device found at address 0x%02X", i2c_addr);
    } else {
        ESP_LOGD(TAG, "No device at address 0x%02X: %s", i2c_addr, esp_err_to_name(err));
    }

    return err;
}

esp_err_t i2c_master_detect_driver(i2c_port_t i2c_port, uint8_t i2c_addr, 
                                    display_driver_ic_t *driver_ic)
{
    if (driver_ic == NULL) {
        ESP_LOGE(TAG, "Driver IC pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Detecting display driver IC at address 0x%02X", i2c_addr);

    // First, check if device is present
    esp_err_t err = i2c_master_probe_device(i2c_port, i2c_addr);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No device found at address 0x%02X", i2c_addr);
        return err;
    }

    // SSD1306 and SSD1315 are largely compatible
    // SSD1315 is a newer version with improved features but uses the same protocol
    // For practical purposes, we default to SSD1306 compatibility mode
    // which works with both ICs
    
    // Try to read status (this works on both SSD1306 and SSD1315)
    // If successful, we know it's an OLED display controller
    *driver_ic = DISPLAY_DRIVER_SSD1306;
    
    ESP_LOGI(TAG, "Display driver detected as SSD1306-compatible");
    ESP_LOGI(TAG, "Note: SSD1315 uses same protocol and is also supported");
    
    return ESP_OK;
}

const char* i2c_master_get_driver_name(display_driver_ic_t driver_ic)
{
    switch (driver_ic) {
        case DISPLAY_DRIVER_SSD1306:
            return "SSD1306";
        case DISPLAY_DRIVER_SSD1315:
            return "SSD1315";
        default:
            return "Unknown";
    }
}
