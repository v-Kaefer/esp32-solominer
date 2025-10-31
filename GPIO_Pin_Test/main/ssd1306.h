#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "driver/i2c.h"

typedef struct {
    i2c_port_t i2c_port;
    uint8_t i2c_addr;
    int width;
    int height;
    int pages;
} SSD1306_t;

void i2c_master_init_ssd1306(SSD1306_t *dev, i2c_port_t i2c_port, int width, int height, uint8_t addr);
void ssd1306_clear_screen(SSD1306_t *dev, bool invert);
void ssd1306_contrast(SSD1306_t *dev, int contrast);
void ssd1306_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert);

#endif // __SSD1306_H__