/**
 * @file ssd1306.h
 * @brief SSD1306 OLED Display Driver
 * 
 * This driver provides functions to control an SSD1306-based OLED display
 * via I2C communication. It supports text display, contrast adjustment,
 * and screen clearing operations.
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "driver/i2c.h"

/**
 * @brief SSD1306 device structure
 * 
 * Contains configuration and state information for an SSD1306 display.
 */
typedef struct {
    i2c_port_t i2c_port;  /*!< I2C port number */
    uint8_t i2c_addr;     /*!< I2C address of the display */
    int width;            /*!< Display width in pixels */
    int height;           /*!< Display height in pixels */
    int pages;            /*!< Number of display pages */
} SSD1306_t;

/**
 * @brief Initialize SSD1306 display
 * 
 * @param dev Pointer to SSD1306 device structure
 * @param i2c_port I2C port number
 * @param width Display width in pixels
 * @param height Display height in pixels
 * @param addr I2C address of the display
 */
void i2c_master_init_ssd1306(SSD1306_t *dev, i2c_port_t i2c_port, int width, int height, uint8_t addr);

/**
 * @brief Clear the entire display screen
 * 
 * @param dev Pointer to SSD1306 device structure
 * @param invert If true, display in inverted colors
 */
void ssd1306_clear_screen(SSD1306_t *dev, bool invert);

/**
 * @brief Set display contrast level
 * 
 * @param dev Pointer to SSD1306 device structure
 * @param contrast Contrast value (0-255)
 */
void ssd1306_contrast(SSD1306_t *dev, int contrast);

/**
 * @brief Display text on a specific page
 * 
 * @param dev Pointer to SSD1306 device structure
 * @param page Page number (0-7 for 128x64 display)
 * @param text Text string to display
 * @param text_len Length of text string
 * @param invert If true, display text in inverted colors
 */
void ssd1306_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert);

#endif // __SSD1306_H__