#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "display.h"
#include "display_config.h"

// Include the appropriate driver based on configuration
#if defined(DISPLAY_DRIVER_SSD1306)
#include "ssd1306.h"
#elif defined(DISPLAY_DRIVER_SSD1315)
#include "ssd1315.h"
#else
#error "No display driver selected! Please define DISPLAY_DRIVER_SSD1306 or DISPLAY_DRIVER_SSD1315 in display_config.h"
#endif

static const char *TAG = "DISPLAY";

// Internal display device structure
struct display_device {
    void *driver_handle;  // Pointer to the specific driver's device structure
    int width;
    int height;
    int pages;
};

esp_err_t display_init_i2c(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = DISPLAY_I2C_SDA_GPIO,
        .scl_io_num = DISPLAY_I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = DISPLAY_I2C_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(DISPLAY_I2C_PORT, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(err));
        return err;
    }
    
    err = i2c_driver_install(DISPLAY_I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "I2C initialized on SDA=%d, SCL=%d", DISPLAY_I2C_SDA_GPIO, DISPLAY_I2C_SCL_GPIO);
    return ESP_OK;
}

display_device_t* display_init(void)
{
    display_device_t *dev = (display_device_t*)malloc(sizeof(display_device_t));
    if (!dev) {
        ESP_LOGE(TAG, "Failed to allocate display device");
        return NULL;
    }
    
    dev->width = DISPLAY_WIDTH;
    dev->height = DISPLAY_HEIGHT;
    dev->pages = DISPLAY_HEIGHT / 8;
    
#if defined(DISPLAY_DRIVER_SSD1306)
    ESP_LOGI(TAG, "Initializing SSD1306 display driver");
    SSD1306_t *ssd_dev = (SSD1306_t*)malloc(sizeof(SSD1306_t));
    if (!ssd_dev) {
        ESP_LOGE(TAG, "Failed to allocate SSD1306 device");
        free(dev);
        return NULL;
    }
    i2c_master_init_ssd1306(ssd_dev, DISPLAY_I2C_PORT, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_I2C_ADDR);
    dev->driver_handle = ssd_dev;
    
#elif defined(DISPLAY_DRIVER_SSD1315)
    ESP_LOGI(TAG, "Initializing SSD1315 display driver");
    SSD1315_t *ssd_dev = (SSD1315_t*)malloc(sizeof(SSD1315_t));
    if (!ssd_dev) {
        ESP_LOGE(TAG, "Failed to allocate SSD1315 device");
        free(dev);
        return NULL;
    }
    i2c_master_init_ssd1315(ssd_dev, DISPLAY_I2C_PORT, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_I2C_ADDR);
    dev->driver_handle = ssd_dev;
#endif
    
    ESP_LOGI(TAG, "Display initialized: %dx%d", dev->width, dev->height);
    return dev;
}

void display_clear(display_device_t *dev, bool invert)
{
    if (!dev || !dev->driver_handle) return;
    
#if defined(DISPLAY_DRIVER_SSD1306)
    ssd1306_clear_screen((SSD1306_t*)dev->driver_handle, invert);
#elif defined(DISPLAY_DRIVER_SSD1315)
    ssd1315_clear_screen((SSD1315_t*)dev->driver_handle, invert);
#endif
}

void display_set_contrast(display_device_t *dev, uint8_t contrast)
{
    if (!dev || !dev->driver_handle) return;
    
#if defined(DISPLAY_DRIVER_SSD1306)
    ssd1306_contrast((SSD1306_t*)dev->driver_handle, contrast);
#elif defined(DISPLAY_DRIVER_SSD1315)
    ssd1315_contrast((SSD1315_t*)dev->driver_handle, contrast);
#endif
}

void display_text(display_device_t *dev, int page, const char *text, int text_len, bool invert)
{
    if (!dev || !dev->driver_handle) return;
    
#if defined(DISPLAY_DRIVER_SSD1306)
    ssd1306_display_text((SSD1306_t*)dev->driver_handle, page, (char*)text, text_len, invert);
#elif defined(DISPLAY_DRIVER_SSD1315)
    ssd1315_display_text((SSD1315_t*)dev->driver_handle, page, (char*)text, text_len, invert);
#endif
}

void display_mining_status(display_device_t *dev, float hashrate, uint64_t total_hashes,
                          uint32_t best_difficulty, uint32_t nonce)
{
    if (!dev) return;
    
    char line[32];
    
    // Clear screen and set contrast
    display_clear(dev, false);
    display_set_contrast(dev, DISPLAY_CONTRAST);
    
    // Title
    display_text(dev, 0, "ESP32 BTC Miner", 15, false);
    display_text(dev, 1, "---------------", 15, false);
    
    // Hashrate
    snprintf(line, sizeof(line), "Rate:%.1fH/s", hashrate);
    display_text(dev, 2, line, strlen(line), false);
    
    // Total hashes
    if (total_hashes < 1000000) {
        snprintf(line, sizeof(line), "Total:%lluK", total_hashes / 1000);
    } else if (total_hashes < 1000000000) {
        snprintf(line, sizeof(line), "Total:%lluM", total_hashes / 1000000);
    } else {
        snprintf(line, sizeof(line), "Total:%lluG", total_hashes / 1000000000);
    }
    display_text(dev, 3, line, strlen(line), false);
    
    // Best difficulty
    snprintf(line, sizeof(line), "Best:%lu zeros", best_difficulty);
    display_text(dev, 4, line, strlen(line), false);
    
    // Current nonce
    snprintf(line, sizeof(line), "Nonce:%lu", nonce);
    display_text(dev, 5, line, strlen(line), false);
}
