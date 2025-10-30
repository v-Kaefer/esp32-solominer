# GitHub Copilot Instructions for ESP32-S3 Bitcoin Miner Project

## Hardware Specifications

### Target Device
- **Model**: ESP32-S3-WROOM-1
- **Official Documentation**:
  - [ESP32-S3-WROOM-1 Datasheet](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.html)
  - [ESP32-S3 Datasheet](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf)
  - [ESP32-S3 Development Kits Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/)

### Connected Peripherals
- **Display**: SSD1306 OLED (128x64 pixels)
- **Interface**: I2C
- **I2C Address**: 0x3C

### GPIO Configuration

**Standard ESP32-S3 GPIO Guidelines** (refer to official documentation):
- Follow the [ESP32-S3 Datasheet](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf) for standard GPIO pinout and functionality
- GPIO numbering should match the official ESP32-S3-WROOM-1 module pin assignments

**⚠️ CURRENT BOARD-SPECIFIC NOTE** (Low-quality burn board with incorrect silkscreen):

The current development board has mislabeled pins. This is specific to this low-quality board and will be replaced with proper documentation when using a standard board.

```c
// TEMPORARY: Verified GPIO mapping for current low-quality board
// NOTE: Board silkscreen labels are INCORRECT - these are the actual working pins
#define I2C_MASTER_SDA_IO    15    // Board incorrectly labels this as "GPIO07"
#define I2C_MASTER_SCL_IO    9     // Board incorrectly labels this as "GPIO09"
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000  // 100 kHz - stable and tested
```

When using a proper development board, follow the standard GPIO assignments from the official ESP32-S3 documentation.

### GPIO Usage Guidelines for ESP32-S3

Refer to the official [ESP32-S3 Datasheet](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf) for complete GPIO specifications.

**Reserved/Strapping Pins** (from official documentation - avoid using or be cautious):
- GPIO0: Strapping pin (boot mode selection)
- GPIO45, GPIO46: Strapping pins
- GPIO19, GPIO20: USB D-, D+ (avoid if USB is needed)

**Safe GPIOs for general use** (per ESP32-S3 specifications):
- GPIO1-18, GPIO21 (excluding GPIO0, GPIO19, GPIO20 for USB, and I2C pins GPIO9, GPIO15 currently in use)

## Development Environment

### IDE and Tools
- **IDE**: VSCode with ESP-IDF Extension
- **Framework**: ESP-IDF (Espressif IoT Development Framework)
- **Build System**: CMake via `idf.py`
- **Programming Interface**: ESP-PROG or USB (depending on setup)

### Build Commands
```bash
# Configure the project (first time or after clean)
idf.py menuconfig

# Build the project
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Combined flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

## Coding Standards and Best Practices

### ESP-IDF Specific Patterns

#### 1. Error Handling
Always use ESP-IDF error checking macros:

```c
// For critical operations
ESP_ERROR_CHECK(esp_wifi_init(&cfg));

// For non-critical operations that need handling
esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "I2C config failed: %s", esp_err_to_name(ret));
    return ret;
}
```

#### 2. Logging
Use ESP-IDF logging system with appropriate levels:

```c
static const char *TAG = "MODULE_NAME";

ESP_LOGE(TAG, "Error message");    // Errors
ESP_LOGW(TAG, "Warning message");  // Warnings
ESP_LOGI(TAG, "Info message");     // Information
ESP_LOGD(TAG, "Debug message");    // Debug (only in debug builds)
ESP_LOGV(TAG, "Verbose message");  // Verbose (very detailed)
```

#### 3. FreeRTOS Tasks
Follow FreeRTOS patterns for task creation:

```c
// Create task with proper stack size and priority
xTaskCreatePinnedToCore(
    task_function,      // Task function
    "task_name",        // Task name for debugging
    8192,               // Stack size in bytes
    NULL,               // Task parameters
    5,                  // Priority (0-24, higher = more priority)
    NULL,               // Task handle (optional)
    1                   // Core ID (0 or 1, or tskNO_AFFINITY)
);
```

#### 4. I2C Peripheral Management

**CRITICAL**: Initialize I2C driver only ONCE during boot:

```c
// Good: Single initialization
void app_main(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}

// Bad: Multiple installations without deletion
// This will cause: i2c: i2c driver install error
```

If you need to reconfigure I2C:
```c
// Delete before reinstalling
i2c_driver_delete(I2C_NUM_0);
// Then configure and install again
```

#### 5. Memory Management
- Use `malloc()` and `free()` from FreeRTOS heap
- For DMA-capable memory, use `heap_caps_malloc(size, MALLOC_CAP_DMA)`
- Always check for NULL after allocation
- Free memory when no longer needed

#### 6. WiFi Integration
```c
// Initialize NVS (required for WiFi)
esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
}
ESP_ERROR_CHECK(ret);

// Initialize network stack
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
```

## Project-Specific Patterns

### Bitcoin Mining Implementation
- Uses double SHA-256 hashing (via mbedtls)
- Implements nonce iteration for block mining
- Tracks mining statistics (hashrate, total hashes, best difficulty)
- Uses FreeRTOS task for mining (typically pinned to Core 1)

### Display Updates
- Update display every 2 seconds to avoid I2C bus congestion
- Clear screen before updating to prevent artifacts
- Use appropriate contrast settings (0xFF for maximum brightness)

### Watchdog Management
```c
// Add delays to prevent watchdog timeout in tight loops
if (nonce % 1000 == 0) {
    vTaskDelay(1);  // Yield to other tasks
}
```

## Security Considerations

1. **Never commit WiFi credentials**:
   - Use defines or environment variables
   - Keep credentials in a separate non-committed file
   - Current pattern: `WIFI_SSID` and `WIFI_PASS` macros

2. **Bitcoin addresses**:
   - Validate format before use
   - Document which address is being used and why

3. **Memory safety**:
   - Always bounds-check array access
   - Validate pointer parameters before dereferencing
   - Use `sizeof()` for buffer operations

## Common Pitfalls and Solutions

### Issue: Display not responding on I2C
**Solution**: 
- For standard boards: Follow the GPIO assignments from the ESP32-S3 datasheet
- For current low-quality board: Verify GPIO pins match the tested configuration (SDA=15, SCL=9), not the board silkscreen labels

### Issue: `i2c driver install error`
**Solution**: Call `i2c_driver_delete()` before reinstalling the driver.

### Issue: WiFi panic during GPIO testing
**Solution**: Disable WiFi when testing/probing GPIOs. Initialize WiFi only after hardware setup is complete.

### Issue: Watchdog timeout
**Solution**: Add periodic `vTaskDelay()` calls in long-running loops.

### Issue: Display shows garbage
**Solution**: Ensure SSD1306 device struct is initialized with `i2c_master_init_ssd1306()` before calling display functions.

## Testing and Debugging

### Serial Monitor
- Use `ESP_LOGx` macros for debugging
- Monitor output with `idf.py monitor`
- Use `Ctrl+]` to exit monitor

### I2C Device Scanning
For troubleshooting I2C devices:
```c
for (uint8_t addr = 1; addr < 0x7F; addr++) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
        ESP_LOGI("I2C", "Device found at 0x%02X", addr);
    }
}
```

## References

### Official ESP32-S3 Documentation
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [ESP32-S3 API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/index.html)
- [FreeRTOS API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/freertos.html)

### Peripheral Guides
- [I2C Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2c.html)
- [WiFi Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/network/esp_wifi.html)
- [GPIO & RTC GPIO](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html)

### SSD1306 OLED
- I2C mode, 128x64 resolution
- Default address: 0x3C (some modules support 0x3D)
- Operating voltage: 3.3V compatible

## Code Structure

```
esp32-solominer/
├── CMakeLists.txt          # Main project CMake file
├── README.md               # Project documentation
├── CHANGELOG.md            # Hardware troubleshooting notes
├── main/
│   ├── CMakeLists.txt      # Main component CMake
│   ├── main.c              # Main application code
│   ├── ssd1306.c           # OLED driver implementation
│   └── ssd1306.h           # OLED driver header
└── .gitignore              # Ignore build artifacts and configs
```

## When Making Changes

1. **Always reference official ESP32-S3 documentation** for hardware-specific features
2. **Test on actual hardware** - simulators may not catch I2C or GPIO issues
3. **Check ESP-IDF version compatibility** - APIs may change between versions
4. **Document hardware quirks** - the current low-quality board has mislabeled GPIO pins (to be replaced with proper board)
5. **Use appropriate delays** - hardware needs time to respond
6. **Monitor memory usage** - ESP32-S3 has limited RAM
7. **Test WiFi separately** from other peripherals to isolate issues
8. **Follow official GPIO pinout** - use ESP32-S3 datasheet as the primary reference for pin assignments

## Component Dependencies

Key ESP-IDF components used in this project:
- `freertos` - Real-time operating system
- `esp_wifi` - WiFi functionality
- `nvs_flash` - Non-volatile storage
- `mbedtls` - Cryptography (SHA-256)
- `driver` - I2C, GPIO drivers
- `esp_event` - Event handling
- `lwip` - TCP/IP stack

These are automatically included via ESP-IDF but should be considered when adding new features.
