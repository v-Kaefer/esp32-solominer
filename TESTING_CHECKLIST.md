# Testing Checklist for Display Configuration

This document provides a comprehensive testing checklist for the modular display configuration system.

## Prerequisites

- [ ] ESP-IDF installed and configured
- [ ] ESP32 board (Classic, S2, S3, or C3)
- [ ] OLED display (128x64, I2C interface)
- [ ] USB cable for flashing and serial monitoring
- [ ] Jumper wires for connections

## Hardware Setup Testing

### Test 1: Physical Connections
- [ ] Connect GND to GND
- [ ] Connect VCC to 3.3V (or 5V if display requires it)
- [ ] Connect SCL to configured GPIO (default: GPIO 8 for S3)
- [ ] Connect SDA to configured GPIO (default: GPIO 15 for S3)
- [ ] Verify all connections are secure

### Test 2: Power Check
- [ ] Display backlight turns on when powered
- [ ] Check voltage at VCC pin is correct (3.3V or 5V)
- [ ] No unusual heating of components

## Configuration Testing

### Test 3: SSD1306 Driver (Default)
- [ ] Open `main/display_config.h`
- [ ] Ensure `DISPLAY_DRIVER_SSD1306` is defined
- [ ] Ensure `DISPLAY_DRIVER_SSD1315` is commented out
- [ ] Build project: `idf.py build`
- [ ] Flash to device: `idf.py flash`
- [ ] Monitor output: `idf.py monitor`
- [ ] Verify display shows "ESP32 BTC Miner"
- [ ] Verify numbers update every 2 seconds

### Test 4: SSD1315 Driver
- [ ] Open `main/display_config.h`
- [ ] Comment out `DISPLAY_DRIVER_SSD1306`
- [ ] Uncomment `DISPLAY_DRIVER_SSD1315`
- [ ] Build project: `idf.py build`
- [ ] Flash to device: `idf.py flash`
- [ ] Monitor output: `idf.py monitor`
- [ ] Verify display shows "ESP32 BTC Miner"
- [ ] Verify numbers update every 2 seconds

### Test 5: I2C Address 0x3C
- [ ] In `display_config.h`, set `DISPLAY_I2C_ADDR` to `0x3C`
- [ ] Build and flash
- [ ] Verify display works correctly

### Test 6: I2C Address 0x3D
- [ ] In `display_config.h`, set `DISPLAY_I2C_ADDR` to `0x3D`
- [ ] Build and flash
- [ ] Verify display works correctly (or note which address works)

### Test 7: Different GPIO Pins
If using non-default pins:
- [ ] Update `DISPLAY_I2C_SDA_GPIO` in `display_config.h`
- [ ] Update `DISPLAY_I2C_SCL_GPIO` in `display_config.h`
- [ ] Reconnect wires to new GPIO pins
- [ ] Build and flash
- [ ] Verify display works correctly

### Test 8: Brightness Control
- [ ] Set `DISPLAY_CONTRAST` to `0xFF` (full brightness)
- [ ] Build and flash
- [ ] Note brightness level
- [ ] Set `DISPLAY_CONTRAST` to `0x80` (50% brightness)
- [ ] Build and flash
- [ ] Verify brightness is reduced
- [ ] Set `DISPLAY_CONTRAST` to `0x40` (25% brightness)
- [ ] Build and flash
- [ ] Verify brightness is further reduced

## Functionality Testing

### Test 9: Display Content
- [ ] Line 0 shows: "ESP32 BTC Miner"
- [ ] Line 1 shows: "---------------" (separator)
- [ ] Line 2 shows: "Rate: X.X H/s" (hashrate)
- [ ] Line 3 shows: "Total: XK/M/G" (total hashes)
- [ ] Line 4 shows: "Best: X zeros" (best difficulty)
- [ ] Line 5 shows: "Nonce: X" (current nonce)

### Test 10: Display Updates
- [ ] Display updates every ~2 seconds
- [ ] Hashrate value changes
- [ ] Total hashes increases
- [ ] Nonce value increases
- [ ] Best difficulty updates when better result found

### Test 11: Startup Sequence
- [ ] Display shows "ESP32-S3 Miner" on startup
- [ ] Display shows "Initializing..." message
- [ ] Display shows "WiFi Connecting..." message
- [ ] Display shows "Starting mining!" message
- [ ] Display transitions to mining status after ~7 seconds

## Error Handling Testing

### Test 12: Wrong I2C Address
- [ ] Set incorrect I2C address (e.g., 0x77)
- [ ] Build and flash
- [ ] Check serial output for error message
- [ ] Verify mining continues despite display error
- [ ] Serial log shows: "Display initialization failed, continuing without display"

### Test 13: Wrong GPIO Pins
- [ ] Set non-existent GPIO pins in config
- [ ] Build and flash
- [ ] Check serial output for error message
- [ ] Verify mining continues despite display error

### Test 14: Disconnected Display
- [ ] Start with display working
- [ ] Disconnect display while running
- [ ] Verify miner continues operation
- [ ] Check serial output continues showing stats
- [ ] Reconnect display and reset device
- [ ] Verify display works again

### Test 15: No Display Connected
- [ ] Remove all display connections
- [ ] Build and flash
- [ ] Verify device boots and runs normally
- [ ] Check serial output for "Display initialization failed"
- [ ] Verify mining continues without crash

## Performance Testing

### Test 16: Mining Performance
- [ ] Note hashrate without display
- [ ] Connect display
- [ ] Note hashrate with display
- [ ] Verify hashrate difference is minimal (<5%)
- [ ] Display updates should not cause visible mining slowdown

### Test 17: I2C Speed
- [ ] Test with `DISPLAY_I2C_FREQ_HZ` = 100000 (100kHz)
- [ ] Verify display works correctly
- [ ] Test with `DISPLAY_I2C_FREQ_HZ` = 400000 (400kHz)
- [ ] Verify display works correctly
- [ ] Note any difference in reliability

## Documentation Testing

### Test 18: Quick Start Guide
- [ ] Follow `QUICK_START.md` from scratch
- [ ] Verify all steps are clear and accurate
- [ ] Verify display works following the guide

### Test 19: Configuration Examples
- [ ] Try configuration from `DISPLAY_EXAMPLES.md`
- [ ] Verify example for your ESP32 variant works
- [ ] Verify pin recommendations are correct

### Test 20: Troubleshooting Guide
- [ ] Intentionally create a problem (wrong address, pins, etc.)
- [ ] Follow troubleshooting steps in `DISPLAY_CONFIG.md`
- [ ] Verify troubleshooting steps resolve the issue

## Cross-Platform Testing

### Test 21: ESP32 Classic (if available)
- [ ] Use GPIO 21 (SDA) and GPIO 22 (SCL)
- [ ] Build and flash
- [ ] Verify display works correctly

### Test 22: ESP32-S3 (if available)
- [ ] Use GPIO 15 (SDA) and GPIO 8 (SCL)
- [ ] Build and flash
- [ ] Verify display works correctly

### Test 23: ESP32-C3 (if available)
- [ ] Use GPIO 5 (SDA) and GPIO 6 (SCL)
- [ ] Build and flash
- [ ] Verify display works correctly

## Stress Testing

### Test 24: Long-Running Test
- [ ] Start miner with display
- [ ] Let run for 1 hour
- [ ] Verify display continues updating
- [ ] Check for memory leaks in serial output
- [ ] Verify no display corruption

### Test 25: Repeated Reset Test
- [ ] Reset device 10 times
- [ ] Verify display initializes correctly each time
- [ ] Verify no issues with repeated initialization

## Code Quality Verification

### Test 26: Build Warnings
- [ ] Build project with `-Wall -Wextra`
- [ ] Verify no compiler warnings in display-related code
- [ ] Verify no linker warnings

### Test 27: Memory Usage
- [ ] Check free heap before display init
- [ ] Check free heap after display init
- [ ] Verify memory usage is reasonable (<1KB for display)
- [ ] Verify no memory leaks after multiple updates

## Final Verification

### Test 28: Complete System Test
- [ ] Display works correctly
- [ ] Mining operates normally
- [ ] Serial output shows status
- [ ] WiFi connects successfully
- [ ] All displayed values are accurate
- [ ] No crashes or hangs
- [ ] Documentation is accurate

## Test Results Summary

**Date**: _______________
**Tester**: _______________
**ESP32 Variant**: _______________
**Display Module**: _______________
**Driver Used**: SSD1306 / SSD1315 (circle one)
**I2C Address**: 0x3C / 0x3D (circle one)

**Overall Result**: PASS / FAIL (circle one)

**Notes**:
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________

## Known Issues

Document any issues found during testing:

1. _______________________________________________________________
2. _______________________________________________________________
3. _______________________________________________________________

## Recommendations

Based on testing results:

1. _______________________________________________________________
2. _______________________________________________________________
3. _______________________________________________________________
