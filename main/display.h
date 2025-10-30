#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c.h"

/**
 * Abstract display interface for ESP32 Solo Miner
 * 
 * This provides a unified interface for different OLED displays,
 * making it easy to switch between different driver ICs.
 */

// Display device handle
typedef struct display_device display_device_t;

/**
 * Initialize the I2C bus for the display
 * @return ESP_OK on success
 */
esp_err_t display_init_i2c(void);

/**
 * Initialize the display
 * @return Pointer to display device handle, or NULL on failure
 */
display_device_t* display_init(void);

/**
 * Clear the display screen
 * @param dev Display device handle
 * @param invert If true, clear to white; if false, clear to black
 */
void display_clear(display_device_t *dev, bool invert);

/**
 * Set display contrast/brightness
 * @param dev Display device handle
 * @param contrast Contrast value (0-255)
 */
void display_set_contrast(display_device_t *dev, uint8_t contrast);

/**
 * Display text on a specific page/line
 * @param dev Display device handle
 * @param page Page number (0-7 for 64-height display)
 * @param text Text string to display
 * @param text_len Length of text string
 * @param invert If true, display inverted (white on black)
 */
void display_text(display_device_t *dev, int page, const char *text, int text_len, bool invert);

/**
 * Display formatted mining status
 * @param dev Display device handle
 * @param hashrate Current hashrate in H/s
 * @param total_hashes Total number of hashes computed
 * @param best_difficulty Best difficulty found (leading zeros)
 * @param nonce Current nonce value
 */
void display_mining_status(display_device_t *dev, float hashrate, uint64_t total_hashes, 
                           uint32_t best_difficulty, uint32_t nonce);

#endif // __DISPLAY_H__
