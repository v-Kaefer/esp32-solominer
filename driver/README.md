# I2C Master Driver for SSD1306/SSD1315 Displays

This directory contains the modular I2C master driver implementation for OLED displays using SSD1306 and SSD1315 driver ICs.

## Overview

The I2C master driver provides a clean, modular interface for initializing and communicating with 128x64 LED display modules. It supports both SSD1306 (original) and SSD1315 (improved variant) driver ICs.

## Hardware Specifications

- **Interface**: 4-pin IIC (GND, VCC, SCL, SDA)
- **Voltage Range**: 3V ~ 5V DC
- **Resolution**: 128x64 pixels (configurable)
- **Power Consumption**: Ultra-low power operation
- **Display Characteristics**: High brightness and contrast

## Features

- ✅ Support for both SSD1306 and SSD1315 driver ICs
- ✅ Voltage range validation (3V-5V)
- ✅ Automatic driver IC detection
- ✅ Configurable I2C pins and clock speed
- ✅ Internal pull-up resistor configuration
- ✅ Ultra-low power mode support
- ✅ High brightness/contrast settings

## Files

- `i2c_master.h` - Header file with API definitions and configuration structures
- `i2c_master.c` - Implementation of I2C master driver functions

## Usage Example

```c
#include "driver/i2c_master.h"

// Initialize I2C with default configuration
i2c_master_config_t config = I2C_MASTER_DEFAULT_CONFIG();
ESP_ERROR_CHECK(i2c_master_init(&config));

// Validate operating voltage
if (!i2c_master_validate_voltage(3300)) {
    ESP_LOGW(TAG, "Voltage outside safe range");
}

// Detect display driver IC
display_driver_ic_t driver_ic;
esp_err_t err = i2c_master_detect_driver(I2C_NUM_0, 0x3C, &driver_ic);
if (err == ESP_OK) {
    ESP_LOGI(TAG, "Detected: %s", i2c_master_get_driver_name(driver_ic));
}

// Probe for device presence
if (i2c_master_probe_device(I2C_NUM_0, 0x3C) == ESP_OK) {
    ESP_LOGI(TAG, "Display found at 0x3C");
}
```

## Configuration

### Default I2C Configuration

```c
i2c_master_config_t config = I2C_MASTER_DEFAULT_CONFIG();
// Defaults:
// - I2C port: I2C_NUM_0
// - SDA: GPIO_NUM_15
// - SCL: GPIO_NUM_9
// - Clock: 100kHz
// - Pull-ups enabled
// - Timeout: 1000ms
```

### Custom Configuration

```c
i2c_master_config_t config = {
    .i2c_port = I2C_NUM_1,
    .sda_io_num = GPIO_NUM_21,
    .scl_io_num = GPIO_NUM_22,
    .clk_speed = I2C_MASTER_FREQ_HZ_FAST,  // 400kHz
    .sda_pullup_en = true,
    .scl_pullup_en = true,
    .timeout_ms = 2000
};
ESP_ERROR_CHECK(i2c_master_init(&config));
```

## Display Configuration

```c
display_config_t display_config = DISPLAY_DEFAULT_CONFIG();
// Defaults:
// - Driver IC: SSD1306
// - Address: 0x3C
// - Resolution: 128x64
// - Contrast: 0xCF
// - Low power mode: enabled
```

## Pin Configuration

The default pin configuration can be changed by modifying the configuration structure:

- **SDA (Data)**: Default GPIO_NUM_15
- **SCL (Clock)**: Default GPIO_NUM_9

### Hardware Connections

```
Display Module    ESP32-S3
--------------    --------
GND          <->  GND
VCC          <->  3.3V or 5V (within 3-5V range)
SCL          <->  GPIO9 (or configured pin)
SDA          <->  GPIO15 (or configured pin)
```

**Note**: External pull-up resistors (4.7kΩ typical) are recommended for better reliability, though internal pull-ups are enabled by default.

## SSD1306 vs SSD1315

Both driver ICs are supported and largely compatible:

### SSD1306
- Original OLED driver IC
- Standard features
- Wide compatibility

### SSD1315
- Improved variant of SSD1306
- Enhanced charge pump
- Better power efficiency
- Optimized timing parameters
- Fully compatible with SSD1306 protocol

The driver automatically handles the differences between the two ICs, using optimized initialization sequences for each.

## API Reference

### Initialization Functions

- `esp_err_t i2c_master_init(const i2c_master_config_t *config)` - Initialize I2C master
- `esp_err_t i2c_master_deinit(i2c_port_t i2c_port)` - Deinitialize I2C master

### Validation Functions

- `bool i2c_master_validate_voltage(uint32_t voltage_mv)` - Validate voltage range

### Detection Functions

- `esp_err_t i2c_master_probe_device(i2c_port_t i2c_port, uint8_t i2c_addr)` - Check device presence
- `esp_err_t i2c_master_detect_driver(i2c_port_t i2c_port, uint8_t i2c_addr, display_driver_ic_t *driver_ic)` - Detect driver IC type

### Utility Functions

- `const char* i2c_master_get_driver_name(display_driver_ic_t driver_ic)` - Get driver IC name as string

## Testing

Unit tests are available in `test/test_i2c_master.c` covering:
- Configuration validation
- Voltage range checking
- Driver IC detection
- Constants verification

## Troubleshooting

### Display Not Detected
1. Check physical connections (GND, VCC, SCL, SDA)
2. Verify voltage is within 3-5V range
3. Try alternate I2C address (0x3D instead of 0x3C)
4. Check pull-up resistors (4.7kΩ recommended)

### Display Initialization Fails
1. Ensure I2C driver is initialized before display
2. Verify GPIO pins are not used by other peripherals
3. Try reducing I2C clock speed to 100kHz
4. Check display module specifications

### Poor Display Quality
1. Adjust contrast setting
2. Verify power supply stability
3. Check for electromagnetic interference
4. Try different pre-charge period settings

## References

- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [SSD1315 Datasheet](https://www.solomon-systech.com/product/ssd1315/)
- [ESP-IDF I2C Driver Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)
