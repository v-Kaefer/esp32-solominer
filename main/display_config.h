#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "driver/i2c.h"

/**
 * Display Configuration for ESP32 Solo Miner
 * 
 * This file allows easy configuration of different OLED displays
 * for the miner. Supports SSD1306 and SSD1315 driver ICs.
 */

// Display driver selection
// Uncomment ONE of the following:
#define DISPLAY_DRIVER_SSD1306    // Original driver IC
// #define DISPLAY_DRIVER_SSD1315    // New driver IC (compatible with SSD1306)

// Display dimensions
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// I2C Configuration
// Adjust these pins according to your ESP32 hardware
#define DISPLAY_I2C_PORT       I2C_NUM_0
#define DISPLAY_I2C_SDA_GPIO   15    // GPIO for SDA
#define DISPLAY_I2C_SCL_GPIO   8     // GPIO for SCL
#define DISPLAY_I2C_FREQ_HZ    100000  // 100kHz

// Display I2C address
// Common addresses: 0x3C or 0x3D
#define DISPLAY_I2C_ADDR       0x3C

// Display settings
#define DISPLAY_CONTRAST       0xFF  // 0x00 to 0xFF (brightness)

#endif // __DISPLAY_CONFIG_H__
