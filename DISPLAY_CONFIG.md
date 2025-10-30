# Display Configuration Guide

## Overview

The ESP32 Solo Miner now features a modular display configuration system that supports multiple OLED display drivers. This allows easy switching between different display types without modifying the core mining code.

## Supported Display Drivers

### SSD1306 (Original)
- Most common OLED driver IC
- Widely available and well-supported
- Default driver selection

### SSD1315 (New)
- Newer driver IC compatible with SSD1306
- May come with newer display modules
- Uses similar initialization sequence

## Configuration

All display configuration is centralized in `main/display_config.h`:

### Selecting Display Driver

Uncomment ONE of the following lines in `display_config.h`:

```c
#define DISPLAY_DRIVER_SSD1306    // Original driver IC
// #define DISPLAY_DRIVER_SSD1315    // New driver IC
```

### Display Dimensions

Default: 128x64 pixels

```c
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64
```

### I2C Pin Configuration

Adjust these pins according to your ESP32 hardware wiring:

```c
#define DISPLAY_I2C_SDA_GPIO   15    // GPIO for SDA
#define DISPLAY_I2C_SCL_GPIO   8     // GPIO for SCL
```

**Common ESP32 I2C Pins:**
- ESP32: GPIO 21 (SDA), GPIO 22 (SCL)
- ESP32-S3: GPIO 8 (SCL), GPIO 15 (SDA) - or use available GPIOs
- ESP32-C3: GPIO 5 (SDA), GPIO 6 (SCL)

### I2C Address

Most OLED displays use one of two addresses:

```c
#define DISPLAY_I2C_ADDR       0x3C  // or 0x3D
```

To find your display's I2C address, you can:
1. Check the display module's documentation
2. Use an I2C scanner sketch
3. Try both 0x3C and 0x3D - one should work

### Display Settings

```c
#define DISPLAY_CONTRAST       0xFF  // 0x00 to 0xFF (brightness)
```

## Display Information

The miner displays the following crucial information:

### Line 0-1: Header
- Title: "ESP32 BTC Miner"
- Separator line

### Line 2: Hashrate
- Current mining hashrate in H/s (Hashes per second)
- Updates every 2 seconds

### Line 3: Total Hashes
- Total number of hashes computed since boot
- Displayed in K (thousands), M (millions), or G (billions)

### Line 4: Best Difficulty
- Best difficulty found (number of leading zeros)
- Higher numbers indicate better results

### Line 5: Current Nonce
- Current nonce value being tested
- Increments with each hash attempt

## Hardware Connection

### Standard OLED Module Wiring

```
OLED Display    ESP32
────────────    ─────
GND         ->  GND
VCC         ->  3.3V
SCL         ->  GPIO 8 (or configured SCL pin)
SDA         ->  GPIO 15 (or configured SDA pin)
```

### Notes:
- Use pull-up resistors (4.7kΩ) on SDA and SCL if not built into your module
- Keep I2C wires short (< 30cm recommended) to avoid signal issues
- Some displays work with 5V VCC, but 3.3V is safer for ESP32

## Troubleshooting

### Display Not Working

1. **Check Physical Connections**
   - Verify wiring matches configuration
   - Ensure good contact at all connection points
   - Check power supply (3.3V or 5V as required)

2. **Verify I2C Configuration**
   - Confirm GPIO pins in `display_config.h` match wiring
   - Try alternative I2C address (0x3C vs 0x3D)
   - Enable I2C debug code in `main.c` to scan for devices

3. **Driver Selection**
   - If using SSD1306 doesn't work, try SSD1315
   - Both drivers are largely compatible

4. **Enable Debug Mode**
   - Uncomment the I2C sweep code in `main.c` `app_main()` function
   - This will scan all pin combinations and I2C addresses
   - Check serial output for detected devices

### No Display but Mining Works

The miner will continue working even if the display fails to initialize. Check the serial console for:
- "Display initialization failed, continuing without display"
- Mining statistics are still logged to serial

## Adding New Display Drivers

To add support for a new display driver:

1. Create `new_driver.h` and `new_driver.c` in `main/`
2. Implement the standard interface:
   - `void i2c_master_init_new_driver(NEW_DRIVER_t *dev, ...)`
   - `void new_driver_clear_screen(NEW_DRIVER_t *dev, bool invert)`
   - `void new_driver_contrast(NEW_DRIVER_t *dev, int contrast)`
   - `void new_driver_display_text(NEW_DRIVER_t *dev, int page, char *text, int text_len, bool invert)`

3. Add driver selection to `display_config.h`:
   ```c
   // #define DISPLAY_DRIVER_NEW_DRIVER
   ```

4. Update `display.c` to include and handle the new driver

5. Update `main/CMakeLists.txt` to include new source files

## Display Module Specifications

Compatible with most I2C OLED modules:
- **Resolution:** 128x64 pixels
- **Interface:** I2C (IIC)
- **Pins:** 4-pin (GND, VCC, SCL, SDA)
- **Voltage:** 3V ~ 5V DC
- **Working Temperature:** -30°C ~ 70°C
- **Driver IC:** SSD1306 or SSD1315
- **Duty:** 1/64
- **Colors:** White, Blue, Yellow-Blue (depends on panel)

## Performance Considerations

The display update runs on the main core and updates every 2 seconds:
- Minimal impact on mining performance
- Mining runs on Core 1 for maximum performance
- Display can be disabled by not calling display initialization
- If display fails, mining continues unaffected
