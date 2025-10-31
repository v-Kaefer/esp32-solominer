# GitHub Copilot Instructions for ESP32 SoloMiner

## Project Context

This is an ESP32-S3 Bitcoin solo mining project focused on **education and learning**, NOT profitability.

**Target Hardware:** ESP32-S3-N16R8 (via ESP-PROG)
**IDE:** VSCode with ESP-IDF Explorer extension
**Framework:** ESP-IDF v5.1.2+
**Language:** C and Assembly (Xtensa)

## Key Project Characteristics

### Core Focus Areas
- Learning C and Assembly programming
- Low-level hardware optimization
- Bitcoin mining protocol implementation
- Thermal management and safety
- Modular, secure code design

### Realistic Expectations
- **Educational value:** Primary goal
- **Hashrate:** 10-60 kH/s depending on configuration
- **Profitability:** NONE (block finding ~300,000 years)
- **Purpose:** Learning, lottery-style participation, skill building

## Critical Guidelines for Code Suggestions

### 1. Security & Privacy
- **NEVER suggest hardcoding WiFi credentials** in any source file
- Always use the `config.h` pattern (with `config.h.example` template)
- WiFi code must be conditionally compiled: `#ifdef WIFI_SSID`
- No credentials in version control

### 2. Thermal Management Requirements

**ALWAYS consider thermal impact when suggesting code changes.**

#### Active Cooling (Fan) REQUIRED For:
- Dual-core mining at any clock speed
- Single-core mining at 240 MHz sustained
- Any overclocking scenarios
- Mining arrays (4+ devices)

#### Passive Cooling Sufficient For:
- Single-core at ≤160 MHz
- Low-power/intermittent operation
- LeafMiner mode

#### Temperature Guidelines:
- Safe: < 70°C
- Warning: 70-80°C
- Throttling: 80-85°C
- Critical: > 90°C

### 3. Performance Optimization Techniques

When suggesting SHA-256 or mining optimizations:

**Effective Optimizations:**
- Midstate pre-computation (30-40% speedup)
- Assembly optimization for critical loops
- Dual-core nonce range splitting
- IRAM placement for hot paths: `IRAM_ATTR` attribute
- Cache-friendly data structures
- Compiler flags: `-O3 -funroll-loops -ffast-math`

**Avoid:**
- Over-optimizing display update code
- Adding excessive logging in mining loops
- Complex algorithms that don't fit in cache
- Heap allocations in hot paths

### 4. Hardware Configuration

**Default Pin Configuration:**
```c
// I2C OLED Display
#define I2C_SDA_PIN 15
#define I2C_SCL_PIN 9

// Fan Control (optional)
#define FAN_PWM_PIN 4
#define FAN_TACH_PIN 5  // Optional RPM reading
```

### 5. Mining Architectures

Reference these when suggesting mining-related features:

**NerdMiner:** User-friendly, visual feedback, 20-30 kH/s
**NMMiner:** Maximum performance, 40-60 kH/s, dual-core, requires active cooling
**LeafMiner:** Ultra-low power, 5-15 kH/s, battery/solar operation

### 6. Code Style & Patterns

- Follow existing ESP-IDF patterns and conventions
- Use ESP-IDF logging: `ESP_LOGI()`, `ESP_LOGE()`, etc.
- Prefer stack allocation over heap in hot paths
- Use `const` for read-only data
- Document thermal impact of performance changes
- Include error handling for all I2C, WiFi, network operations

### 7. Common Pitfalls to Avoid

- **Don't** suggest removing thermal throttling code
- **Don't** ignore error return values from ESP-IDF functions
- **Don't** add delays in mining loops
- **Don't** assume profitability or commercial viability
- **Don't** suggest using both cores without mentioning cooling requirements
- **Don't** recommend display updates faster than every 1-2 seconds

### 8. Build System

- Uses CMake with ESP-IDF build system
- Main component in `main/` directory
- Custom components in `driver/` directory
- Build with: `idf.py set-target esp32s3 && idf.py build`

### 9. Testing & CI/CD

- No automated test infrastructure currently exists
- Manual testing via serial monitor required
- All PRs must pass: Build, Static Analysis, CodeQL, Code Quality checks
- Monitor hashrate and temperature during testing

### 10. Documentation References

For comprehensive details, refer to:
- `ESP32_MINING_STRATEGIES.md` - Deep technical guide on mining approaches
- `MINING_QUICKSTART.md` - Quick reference for getting started
- `.github/agents/CONTEXT.md` - Detailed agent context and guidelines

## Suggested Code Patterns

### WiFi Configuration (Secure Pattern)
```c
#ifdef WIFI_SSID
    // Initialize WiFi only if config.h exists
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
#endif
```

### IRAM Optimization for Hot Functions
```c
IRAM_ATTR static inline void sha256_transform(/* params */) {
    // Critical mining code in IRAM for speed
}
```

### Temperature Monitoring
```c
// Example pattern for temperature monitoring
float temp = read_temperature();
if (temp > 80.0f) {
    ESP_LOGW(TAG, "High temperature: %.1f°C - Consider cooling", temp);
    // Implement throttling or warn user
}
```

### Dual-Core Mining Pattern
```c
// Core 0: Mine nonce range 0x00000000 - 0x7FFFFFFF
// Core 1: Mine nonce range 0x80000000 - 0xFFFFFFFF
xTaskCreatePinnedToCore(mining_task, "mine0", 4096, (void*)0, 5, NULL, 0);
xTaskCreatePinnedToCore(mining_task, "mine1", 4096, (void*)1, 5, NULL, 1);
```

## When Making Suggestions

1. **Always mention thermal implications** for performance changes
2. **Reference cooling requirements** when suggesting dual-core or overclocking
3. **Emphasize educational value** over profitability claims
4. **Use existing ESP-IDF APIs** - don't reinvent what framework provides
5. **Consider power consumption** for battery/solar scenarios
6. **Validate I2C operations** - displays can be flaky, handle errors
7. **Set realistic expectations** about hashrate and block finding probability

## Project Goals Reminder

This project is about:
- ✅ Learning Bitcoin protocol and mining algorithms
- ✅ Mastering low-level C/Assembly programming
- ✅ Hardware optimization and thermal management
- ✅ Building something cool and educational
- ❌ NOT about making money or competing with ASICs

## Additional Resources

- Bitcoin Whitepaper: bitcoin.org/bitcoin.pdf
- ESP-IDF Documentation: docs.espressif.com
- Stratum V1 Protocol: braiins.com/stratum-v1/docs
- ESP32-S3 Technical Reference: Available from Espressif
