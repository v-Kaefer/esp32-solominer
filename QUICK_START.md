# Quick Start Guide - Display Configuration

This is a quick reference for configuring your OLED display for the ESP32 Solo Miner.

## Step 1: Choose Your Display Driver

Open `main/display_config.h` and uncomment ONE line:

```c
#define DISPLAY_DRIVER_SSD1306    // For SSD1306 displays
// #define DISPLAY_DRIVER_SSD1315    // For SSD1315 displays
```

> **Tip**: Most displays use SSD1306. If that doesn't work, try SSD1315.

## Step 2: Set Your I2C Pins

Edit these lines to match your wiring:

```c
#define DISPLAY_I2C_SDA_GPIO   15    // Your SDA GPIO number
#define DISPLAY_I2C_SCL_GPIO   8     // Your SCL GPIO number
```

### Common Pin Configurations:

| ESP32 Type | SDA Pin | SCL Pin |
|------------|---------|---------|
| ESP32 Classic | GPIO 21 | GPIO 22 |
| ESP32-S3 | GPIO 15 | GPIO 8 |
| ESP32-C3 | GPIO 5 | GPIO 6 |

## Step 3: Set I2C Address (if needed)

Most displays use `0x3C`, but some use `0x3D`:

```c
#define DISPLAY_I2C_ADDR       0x3C  // Try 0x3D if 0x3C doesn't work
```

## Step 4: Build and Flash

```bash
idf.py build
idf.py flash
```

## Troubleshooting

### Display not working?

1. **Check wiring**: Ensure SDA and SCL are connected correctly
2. **Try other driver**: Switch between SSD1306 and SSD1315
3. **Try other address**: Switch between 0x3C and 0x3D
4. **Check power**: Ensure display has 3.3V or 5V (check your display specs)

### Display is blank

- Lower the I2C frequency in display_config.h:
  ```c
  #define DISPLAY_I2C_FREQ_HZ    100000  // 100kHz (slower but more reliable)
  ```

### Display is too bright

- Reduce contrast in display_config.h:
  ```c
  #define DISPLAY_CONTRAST       0x80  // 50% brightness
  ```

## What You'll See

Once configured correctly, your display will show:

```
ESP32 BTC Miner
---------------
Rate: 15.3 H/s
Total: 45K
Best: 12 zeros
Nonce: 123456
```

## Need More Help?

- See [DISPLAY_CONFIG.md](DISPLAY_CONFIG.md) for detailed documentation
- See [DISPLAY_EXAMPLES.md](DISPLAY_EXAMPLES.md) for example configurations
- Check serial output for error messages and I2C debugging info

## Hardware Connection

```
OLED Display    ESP32
────────────    ─────────────────
GND         ->  GND
VCC         ->  3.3V (or 5V)
SCL         ->  Your SCL GPIO
SDA         ->  Your SDA GPIO
```

## Success Indicators

✅ Display shows "ESP32 BTC Miner" on startup
✅ Numbers update every 2 seconds
✅ Hashrate is non-zero

If mining works but display doesn't, the miner will continue operating normally. Display issues won't affect mining performance.
