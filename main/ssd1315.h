#ifndef __SSD1315_H__
#define __SSD1315_H__

#include "driver/i2c.h"

/**
 * SSD1315 OLED Display Driver
 * 
 * SSD1315 is a newer driver IC that is largely compatible with SSD1306
 * but may have slightly different initialization sequences or settings.
 */

typedef struct {
    i2c_port_t i2c_port;
    uint8_t i2c_addr;
    int width;
    int height;
    int pages;
} SSD1315_t;

void i2c_master_init_ssd1315(SSD1315_t *dev, i2c_port_t i2c_port, int width, int height, uint8_t addr);
void ssd1315_clear_screen(SSD1315_t *dev, bool invert);
void ssd1315_contrast(SSD1315_t *dev, int contrast);
void ssd1315_display_text(SSD1315_t *dev, int page, char *text, int text_len, bool invert);

#endif // __SSD1315_H__
