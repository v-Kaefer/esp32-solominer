#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1315.h"

static const char *TAG = "SSD1315";

#define OLED_CMD_SET_CONTRAST        0x81
#define OLED_CMD_DISPLAY_RAM         0xA4
#define OLED_CMD_DISPLAY_ALLON       0xA5
#define OLED_CMD_DISPLAY_NORMAL      0xA6
#define OLED_CMD_DISPLAY_INVERTED    0xA7
#define OLED_CMD_DISPLAY_OFF         0xAE
#define OLED_CMD_DISPLAY_ON          0xAF
#define OLED_CMD_SET_MEMORY_ADDR_MODE 0x20
#define OLED_CMD_SET_COLUMN_RANGE    0x21
#define OLED_CMD_SET_PAGE_RANGE      0x22

// 5x8 font (simple ASCII)
static const uint8_t font5x8[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

static esp_err_t ssd1315_write_command(SSD1315_t *dev, uint8_t command) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true); // Control byte for command
    i2c_master_write_byte(cmd, command, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t ssd1315_write_data(SSD1315_t *dev, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x40, true); // Data mode
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void i2c_master_init_ssd1315(SSD1315_t *dev, i2c_port_t i2c_port, int width, int height, uint8_t addr) {
    dev->i2c_port = i2c_port;
    dev->i2c_addr = addr;
    dev->width = width;
    dev->height = height;
    dev->pages = height / 8;

    ESP_LOGI(TAG, "Initializing SSD1315 at I2C addr 0x%02X", addr);

    // SSD1315 initialization sequence (very similar to SSD1306)
    // The main difference is that SSD1315 may have slightly different default settings
    ssd1315_write_command(dev, OLED_CMD_DISPLAY_OFF);
    ssd1315_write_command(dev, 0xD5); // Set display clock divide
    ssd1315_write_command(dev, 0x80);
    ssd1315_write_command(dev, 0xA8); // Set multiplex
    ssd1315_write_command(dev, height - 1);
    ssd1315_write_command(dev, 0xD3); // Set display offset
    ssd1315_write_command(dev, 0x00);
    ssd1315_write_command(dev, 0x40); // Set start line
    ssd1315_write_command(dev, 0x8D); // Charge pump
    ssd1315_write_command(dev, 0x14);
    ssd1315_write_command(dev, OLED_CMD_SET_MEMORY_ADDR_MODE);
    ssd1315_write_command(dev, 0x00); // Horizontal addressing mode
    ssd1315_write_command(dev, 0xA1); // Segment remap
    ssd1315_write_command(dev, 0xC8); // COM scan direction
    ssd1315_write_command(dev, 0xDA); // Set COM pins
    ssd1315_write_command(dev, 0x12);
    ssd1315_write_command(dev, OLED_CMD_SET_CONTRAST);
    ssd1315_write_command(dev, 0xCF);
    ssd1315_write_command(dev, 0xD9); // Set precharge
    ssd1315_write_command(dev, 0xF1);
    ssd1315_write_command(dev, 0xDB); // Set VCOM detect
    ssd1315_write_command(dev, 0x40);
    ssd1315_write_command(dev, OLED_CMD_DISPLAY_RAM);
    ssd1315_write_command(dev, OLED_CMD_DISPLAY_NORMAL);
    ssd1315_write_command(dev, OLED_CMD_DISPLAY_ON);
    
    // Small delay to let display stabilize after initialization
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "SSD1315 initialization complete");
}

void ssd1315_clear_screen(SSD1315_t *dev, bool invert) {
    uint8_t pattern = invert ? 0xFF : 0x00;
    uint8_t buffer[128];
    memset(buffer, pattern, sizeof(buffer));
    
    for (int page = 0; page < dev->pages; page++) {
        ssd1315_write_command(dev, 0xB0 + page); // Set page
        ssd1315_write_command(dev, 0x00); // Set lower column
        ssd1315_write_command(dev, 0x10); // Set higher column
        ssd1315_write_data(dev, buffer, dev->width);
    }
}

void ssd1315_contrast(SSD1315_t *dev, int contrast) {
    ssd1315_write_command(dev, OLED_CMD_SET_CONTRAST);
    ssd1315_write_command(dev, contrast);
}

void ssd1315_display_text(SSD1315_t *dev, int page, char *text, int text_len, bool invert) {
    if (page >= dev->pages) return;
    
    uint8_t buffer[128];
    memset(buffer, invert ? 0xFF : 0x00, sizeof(buffer));
    
    int x = 0;
    for (int i = 0; i < text_len && x < dev->width; i++) {
        char c = text[i];
        if (c < 32 || c > 90) c = 32; // Space for unsupported chars
        
        const uint8_t *glyph = font5x8[c - 32];
        for (int j = 0; j < 5 && x < dev->width; j++) {
            buffer[x++] = invert ? ~glyph[j] : glyph[j];
        }
        if (x < dev->width) {
            buffer[x++] = invert ? 0xFF : 0x00; // Space between chars
        }
    }
    
    ssd1315_write_command(dev, 0xB0 + page);
    ssd1315_write_command(dev, 0x00);
    ssd1315_write_command(dev, 0x10);
    ssd1315_write_data(dev, buffer, dev->width);
}
