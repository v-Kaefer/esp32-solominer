# GPIO Pin Test Tool

This is a standalone ESP-IDF project for testing and identifying I2C GPIO pin configurations on ESP32 boards.

## Purpose

This tool helps you identify the correct SDA and SCL pins for I2C communication on your ESP32 board. It's particularly useful when:
- You're unsure which pins are connected to I2C devices
- You want to verify I2C device addresses
- You're debugging I2C connectivity issues

## Features

- **Full Pin Sweep**: Tests all possible pin combinations to find working I2C pairs
- **Plausible Pair Testing**: Quickly test specific pin combinations you suspect might work
- **I2C Address Scanning**: Scans all I2C addresses (0x01-0x7E) to find connected devices
- **OLED Detection**: Specifically tests for common OLED display addresses (0x3C, 0x3D)

## How to Use

1. **Build the project:**
   ```bash
   cd GPIO_Pin_Test
   idf.py build
   ```

2. **Flash to your ESP32:**
   ```bash
   idf.py flash monitor
   ```

3. **Choose your test method** by editing `main/main.c`:
   - For a comprehensive sweep: Use `i2c_pin_sweep()` (default)
   - For targeted testing: Use `test_plausible_pairs()` and adjust the candidate arrays

4. **Read the output:**
   - The tool will log all attempts and successful connections
   - Look for "FOUND" messages indicating successful I2C communication
   - Note the SDA/SCL pins and device addresses that work

## Functions

### `i2c_pin_sweep()`
Systematically tests all possible SDA/SCL pin combinations from the candidate list. This is thorough but time-consuming.

### `test_plausible_pairs()`
Tests only specific pin combinations you define. Faster when you have a good idea of the correct pins.

### `try_pair(sda, scl)`
Tests a specific SDA/SCL pair for OLED displays at common addresses (0x3C and 0x3D).

### `quick_probe_on_pins(sda, scl, addr)`
Quickly checks if a specific I2C device address responds on given pins.

## Customization

Edit the following arrays in `main/main.c` to match your board:

```c
// ESP32-S3 pin candidates
static const int s3_i2c_candidates[] = {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21
};

// For targeted testing
int sda_candidates[] = {8, 15, 7};      // Your suspected SDA pins
int scl_candidates[] = {1, 2, 3, 4, 9}; // Your suspected SCL pins
```

## Notes

- Some pins are automatically skipped (USB pins 19-20, input-only pin 46)
- The tool uses 100kHz I2C speed for reliable testing
- Internal pull-ups are enabled for all tested pins
- A small delay between tests helps stabilize the output

## Interpreting Results

- **"FOUND: SDA=X SCL=Y | N device(s)"**: Successful I2C communication found
- **"addr 0x3C"** or **"addr 0x3D"**: Common OLED display addresses
- **">> FOUND ACK!"**: Device responded to probe at specified address
- **"nothing found"**: No device detected on this pin pair

## Original Source

These test functions were extracted from the main esp32-solominer project to:
- Reduce security alerts about unused code
- Provide a dedicated testing tool
- Keep the main mining code focused and clean
