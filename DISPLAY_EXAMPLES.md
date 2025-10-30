# Display Configuration Examples

This file contains example configurations for common OLED display setups.
Copy the relevant section to your `main/display_config.h` file.

## Example 1: Standard 128x64 OLED with SSD1306 on ESP32-S3

```c
#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "driver/i2c.h"

// Display driver selection
#define DISPLAY_DRIVER_SSD1306    // Original driver IC
// #define DISPLAY_DRIVER_SSD1315    // New driver IC

// Display dimensions
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// I2C Configuration for ESP32-S3
#define DISPLAY_I2C_PORT       I2C_NUM_0
#define DISPLAY_I2C_SDA_GPIO   15    // SDA on GPIO15
#define DISPLAY_I2C_SCL_GPIO   8     // SCL on GPIO8
#define DISPLAY_I2C_FREQ_HZ    100000  // 100kHz

// Display I2C address
#define DISPLAY_I2C_ADDR       0x3C

// Display settings
#define DISPLAY_CONTRAST       0xFF  // Full brightness

#endif // __DISPLAY_CONFIG_H__
```

## Example 2: 128x64 OLED with SSD1315 on ESP32 (Classic)

```c
#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "driver/i2c.h"

// Display driver selection
// #define DISPLAY_DRIVER_SSD1306    // Original driver IC
#define DISPLAY_DRIVER_SSD1315    // New driver IC

// Display dimensions
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// I2C Configuration for ESP32 Classic
#define DISPLAY_I2C_PORT       I2C_NUM_0
#define DISPLAY_I2C_SDA_GPIO   21    // SDA on GPIO21 (standard)
#define DISPLAY_I2C_SCL_GPIO   22    // SCL on GPIO22 (standard)
#define DISPLAY_I2C_FREQ_HZ    100000  // 100kHz

// Display I2C address (try 0x3D if 0x3C doesn't work)
#define DISPLAY_I2C_ADDR       0x3C

// Display settings
#define DISPLAY_CONTRAST       0xFF  // Full brightness

#endif // __DISPLAY_CONFIG_H__
```

## Example 3: Alternative I2C Address (0x3D)

Some displays use address 0x3D instead of 0x3C:

```c
#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "driver/i2c.h"

// Display driver selection
#define DISPLAY_DRIVER_SSD1306    // Original driver IC
// #define DISPLAY_DRIVER_SSD1315    // New driver IC

// Display dimensions
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// I2C Configuration
#define DISPLAY_I2C_PORT       I2C_NUM_0
#define DISPLAY_I2C_SDA_GPIO   15
#define DISPLAY_I2C_SCL_GPIO   8
#define DISPLAY_I2C_FREQ_HZ    100000

// Alternative I2C address
#define DISPLAY_I2C_ADDR       0x3D  // Changed from 0x3C

// Display settings
#define DISPLAY_CONTRAST       0xFF

#endif // __DISPLAY_CONFIG_H__
```

## Example 4: Lower Brightness Configuration

If your display is too bright:

```c
#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "driver/i2c.h"

// Display driver selection
#define DISPLAY_DRIVER_SSD1306
// #define DISPLAY_DRIVER_SSD1315

// Display dimensions
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// I2C Configuration
#define DISPLAY_I2C_PORT       I2C_NUM_0
#define DISPLAY_I2C_SDA_GPIO   15
#define DISPLAY_I2C_SCL_GPIO   8
#define DISPLAY_I2C_FREQ_HZ    100000

// Display I2C address
#define DISPLAY_I2C_ADDR       0x3C

// Display settings - reduced brightness
#define DISPLAY_CONTRAST       0x80  // 50% brightness (0x00-0xFF)

#endif // __DISPLAY_CONFIG_H__
```

## Example 5: Faster I2C Speed (400kHz)

For shorter cables and better quality displays:

```c
#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "driver/i2c.h"

// Display driver selection
#define DISPLAY_DRIVER_SSD1306
// #define DISPLAY_DRIVER_SSD1315

// Display dimensions
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// I2C Configuration - Fast Mode
#define DISPLAY_I2C_PORT       I2C_NUM_0
#define DISPLAY_I2C_SDA_GPIO   15
#define DISPLAY_I2C_SCL_GPIO   8
#define DISPLAY_I2C_FREQ_HZ    400000  // 400kHz Fast Mode

// Display I2C address
#define DISPLAY_I2C_ADDR       0x3C

// Display settings
#define DISPLAY_CONTRAST       0xFF

#endif // __DISPLAY_CONFIG_H__
```

## Troubleshooting Tips

### Display Not Working?

1. **Try the other driver**: If SSD1306 doesn't work, try SSD1315
2. **Try the other address**: Switch between 0x3C and 0x3D
3. **Check your wiring**: Make sure SDA and SCL are connected correctly
4. **Reduce I2C speed**: Try 100000 Hz instead of 400000 Hz
5. **Enable debug scanning**: Uncomment the I2C sweep code in main.c

### Common GPIO Pins by Board

**ESP32 Classic:**
- Default I2C: GPIO21 (SDA), GPIO22 (SCL)
- Alternative: GPIO4 (SDA), GPIO5 (SCL)

**ESP32-S2:**
- Recommended: GPIO33 (SDA), GPIO34 (SCL)
- Alternative: Any available GPIO

**ESP32-S3:**
- Flexible: Any GPIO (except USB pins 19, 20)
- Example: GPIO15 (SDA), GPIO8 (SCL)

**ESP32-C3:**
- Recommended: GPIO5 (SDA), GPIO6 (SCL)
- Alternative: GPIO1 (SDA), GPIO2 (SCL)
