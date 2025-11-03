# ESP32 SoloMiner - Agent Context & Instructions

This document provides essential context for AI agents working on the ESP32 SoloMiner project.

## Project Overview

**ESP32 SoloMiner** is a learning-focused Bitcoin mining project for ESP32-S3 microcontrollers. The project emphasizes:
- Educational value over profitability
- Pure software-based mining (no external ASIC)
- Low-level C and Assembly programming
- Hardware optimization and thermal management
- Safety, privacy, and modularity

**Target Hardware:** ESP32-S3-N16R8 (via ESP-PROG)
**IDE:** VSCode with ESP-IDF Explorer extension
**Framework:** ESP-IDF v5.1.2+

## Mining Strategy Documentation

The repository contains comprehensive mining strategy documentation that MUST be consulted when working on mining-related features:

### Primary Documents
1. **[ESP32_MINING_STRATEGIES.md](../../ESP32_MINING_STRATEGIES.md)** - Comprehensive technical guide
2. **[MINING_QUICKSTART.md](../../MINING_QUICKSTART.md)** - Quick reference guide

### Key Mining Approaches Documented

#### 1. NerdMiner Architecture
- **Focus:** User-friendly, visual feedback
- **Hashrate:** 20-30 kH/s (240MHz)
- **Cooling:** Active recommended for 240MHz
- **Best For:** Beginners, learning, visual mining experience
- **Features:** OLED display, WiFi-based work fetching, Stratum v1, web config

#### 2. NMMiner Architecture
- **Focus:** Maximum performance
- **Hashrate:** 40-60 kH/s (dual-core, 240MHz)
- **Cooling:** Active cooling **MANDATORY**
- **Best For:** Experienced users, maximum hashrate
- **Features:** Dual-core mining, assembly optimizations, minimal overhead, thermal monitoring

#### 3. LeafMiner Architecture
- **Focus:** Ultra-low power, battery/solar operation
- **Hashrate:** 5-15 kH/s (intermittent)
- **Cooling:** Passive only (low power operation)
- **Best For:** Off-grid, battery-powered, solar projects
- **Features:** Deep sleep cycles, power management, e-paper display option

## Critical Thermal Management Requirements

**ALWAYS advise on cooling requirements when working with ESP32 mining code:**

### Active Cooling Required:
- ✅ Dual-core mining at any clock speed
- ✅ Single-core mining at 240 MHz sustained operation
- ✅ Mining arrays (4+ devices)
- ✅ Overclocked configurations

### Passive Cooling Sufficient:
- ✅ Single-core at ≤160 MHz
- ✅ LeafMiner low-power mode
- ✅ Intermittent operation

### Temperature Guidelines:
- **Safe:** < 70°C
- **Warning:** 70-80°C
- **Throttling:** 80-85°C
- **Critical:** > 90°C

### Fan Specifications:
- **Size:** 25mm or 40mm DC fan
- **Voltage:** 5V
- **CFM:** 5-10+ CFM minimum
- **Power:** 0.1-0.3W additional
- **Control:** PWM optional for variable speed

## Performance Expectations

| Configuration | Clock | Cores | Cooling | Hashrate | Power |
|--------------|-------|-------|---------|----------|-------|
| Basic Solo | 160MHz | 1 | Passive | 10-15 kH/s | 0.8W |
| Standard | 240MHz | 1 | Active | 20-30 kH/s | 1.2W |
| Optimized | 240MHz | 2 | Active | 40-50 kH/s | 1.8W |
| Overclocked | 240MHz+ | 2 | Active | 50-60 kH/s | 2.0W |
| Low Power | 80MHz | 1 | Passive | 5-10 kH/s | 0.3W |

**Reality Check:** ESP32 mining is NOT profitable. Expected time to find a Bitcoin block: ~300,000 years.

**Value Proposition:**
- Educational (learning Bitcoin protocol, mining algorithms)
- Lottery-style participation (non-zero chance of finding block)
- Hardware optimization skills
- Network participation

## Hardware Configuration

### Minimum Setup (~$15)
- ESP32-S3-DevKitC-1 (N16R8)
- SSD1306 OLED Display (128x64, I2C)
- USB-C cable
- Breadboard + jumper wires

### Recommended Setup (~$20-25)
- ESP32-S3-DevKitC-1 (N16R8)
- SSD1306 OLED or ST7789 TFT
- **40mm 5V DC fan (active cooling)**
- Aluminum heatsink with thermal paste
- USB-C 5V/2-3A power supply

### Pin Configuration (Default)
```
I2C OLED:
  SDA: GPIO 15
  SCL: GPIO 9
  VCC: 3.3V
  GND: GND

Fan Control:
  PWM: GPIO 4 (or any PWM-capable GPIO)
  TACH: GPIO 5 (optional, for RPM reading)
```

## Code Optimization Techniques

### SHA-256 Optimization
1. **Midstate Pre-computation:** Pre-compute first SHA-256 round (~30-40% speedup)
2. **Assembly Optimization:** Use Xtensa-specific instructions for critical loops
3. **Dual-Core Mining:** Split nonce range across both cores
4. **IRAM Placement:** Keep hot code paths in instruction RAM
5. **Cache Optimization:** Minimize cache misses with proper data structures

### Compiler Flags
```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funroll-loops")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
```

### Memory Management
- Use IRAM for hot code paths
- Minimize heap allocations
- Stack-based temporary buffers
- Efficient nonce incrementing

## WiFi Configuration (Security)

**NEVER commit WiFi credentials to the repository.**

The project uses a configuration pattern:
1. `main/config.h.example` - Template committed to repo
2. `main/config.h` - Local config (gitignored, contains actual credentials)
3. CI/CD builds skip WiFi functionality (conditional compilation)

```c
#ifdef WIFI_SSID
// WiFi code only compiled when config.h exists
#endif
```

## Network & Pool Integration

### Stratum V1 Protocol
- Standard protocol for pool mining
- Work fetching and share submission
- Session management and authorization

### Recommended Pools
- **Solo Mining:** solo.ckpool.org (lottery-style)
- **Pool Mining:** Braiins Pool, NiceHash (regular tiny payouts)

## Build & Development

### Building
```bash
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

### CI/CD Requirements
All PRs must pass:
- ✅ Build verification
- 🔍 Static code analysis  
- 🔒 Security scanning (CodeQL)
- 📝 Code quality checks

### Testing
- No automated test infrastructure currently exists
- Manual testing required
- Monitor serial output for hashrate and errors

## Common Issues & Solutions

### Overheating
**Symptoms:** Crashes, hashrate drops, instability
**Solutions:**
1. Add active cooling (fan)
2. Reduce clock speed (240MHz → 160MHz)
3. Add heatsink with thermal paste
4. Enable thermal throttling in code
5. Use single-core instead of dual-core

### Low Hashrate
**Expected:** 10-15 kH/s @ 160MHz single-core, 40-50 kH/s @ 240MHz dual-core
**Check:**
1. CPU frequency (should be 240MHz)
2. Both cores mining (dual-core mode)
3. SHA-256 optimization enabled
4. No thermal throttling
5. Debug logging disabled in mining loop

### Display Issues
**Check:**
1. I2C pins (SDA/SCL)
2. I2C address (0x3C or 0x3D)
3. Pull-up resistors (4.7kΩ)
4. Power supply stability
5. Reduce display update frequency

## Future Hardware Acceleration

**ESP32-S3 Hardware SHA Accelerator:**
- ESP32-S3 has built-in SHA hardware acceleration
- Potential 2-5x speedup for SHA-256
- Not yet implemented in this project
- Future enhancement: 100-200 kH/s possible

## Agent Guidelines

When working on this repository:

1. **Always consult mining strategy docs** before implementing mining features
2. **Advise on cooling** when code changes affect thermal load
3. **Set realistic expectations** - emphasize educational value, not profit
4. **Maintain security** - never commit credentials
5. **Follow existing patterns** - WiFi conditional compilation, config.h approach
6. **Document thermal impact** - note if changes increase heat generation
7. **Reference community projects** - NerdMiner, NMMiner, LeafMiner when relevant
8. **Optimize carefully** - balance performance with stability and safety

## Project Goals Reminder

This is a **learning project** focused on:
- Learning C and Assembly programming
- Developing low-level, hardware-effective code
- Ensuring safety, privacy, and modularity
- Building an ESP32 Bitcoin miner from scratch
- **NOT focused on profitability or commercial viability**

## Additional Resources

- **Bitcoin Whitepaper:** bitcoin.org/bitcoin.pdf
- **ESP-IDF Docs:** docs.espressif.com
- **ESP32-S3 Technical Reference:** Available from Espressif
- **Stratum Protocol:** braiins.com/stratum-v1/docs
- **Community:** r/esp32, r/BitcoinMining, NerdMiner Discord

---

**Last Updated:** 2025-10-31  
**Version:** 1.0  
**Maintained By:** Project contributors

For the most current and detailed information, always refer to:
- [ESP32_MINING_STRATEGIES.md](../../ESP32_MINING_STRATEGIES.md)
- [MINING_QUICKSTART.md](../../MINING_QUICKSTART.md)

## Historical Implementation Notes

For detailed technical implementation notes about past features and fixes, see:
- [implementation-notes/](implementation-notes/) - Contains detailed I2C driver updates, test infrastructure implementation, and diagnostic notes
