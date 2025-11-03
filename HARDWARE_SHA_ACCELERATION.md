# ESP32-S3 Hardware SHA Acceleration

## Overview

This project uses the ESP32-S3's built-in hardware SHA accelerator via mbedTLS's direct SHA256 API for Bitcoin mining performance.

## Implementation Approach

This implementation uses mbedTLS SHA256 API which automatically leverages ESP-IDF's hardware SHA acceleration in ESP-IDF v5.4:

1. **mbedTLS SHA256 Direct API**: Uses `mbedtls_sha256()` which automatically uses hardware acceleration
2. **Simple Function Calls**: No context management needed - straightforward API
3. **Hardware CLZ Instruction**: Uses `__builtin_clz` for faster leading zero counting

### Performance Impact

Expected performance with hardware SHA:
- **Software SHA (baseline)**: ~10-20 kH/s
- **Hardware SHA (this implementation)**: ~30-60 kH/s (2-4x improvement)
- Actual performance depends on clock speed, core count, and thermal conditions

## Configuration

Hardware SHA acceleration is enabled through `sdkconfig.defaults`:

```
CONFIG_MBEDTLS_HARDWARE_SHA=y
CONFIG_COMPILER_OPTIMIZATION_PERF=y
```

### Verifying Hardware Acceleration

When the device boots, check the serial monitor output:

```
I (xxx) BTC_MINER: Hardware SHA acceleration: ENABLED
I (xxx) BTC_MINER: Using ESP32-S3 hardware SHA accelerator (mbedtls_sha256)
```

If you see "DISABLED", the configuration is not active and you'll be using software SHA.

## Technical Details

### Implementation

The `double_sha256()` function uses mbedTLS's direct SHA256 API:

```c
#include "mbedtls/sha256.h"

void double_sha256(const uint8_t* data, size_t len, uint8_t* hash)
{
    // First SHA256 - uses hardware accelerator automatically
    uint8_t temp[32];
    mbedtls_sha256(data, len, temp, 0); // 0 = SHA256 (not SHA224)
    
    // Second SHA256 - uses hardware accelerator automatically
    mbedtls_sha256(temp, 32, hash, 0);
}
```

**Why This Works**: In ESP-IDF v5.4, `mbedtls_sha256()` automatically uses the ESP32-S3 hardware SHA peripheral when `CONFIG_MBEDTLS_HARDWARE_SHA=y` is set. No context management needed - the function handles everything internally.

When `CONFIG_MBEDTLS_HARDWARE_SHA=y` is set, mbedTLS automatically routes SHA-256 operations through the ESP32-S3's hardware SHA peripheral instead of software implementation.

### Hardware SHA Peripheral

The ESP32-S3 SHA peripheral:
- Supports SHA-1, SHA-224, SHA-256, SHA-384, and SHA-512
- Operates independently from CPU cores
- Can process data in parallel with CPU operations
- Provides DMA support for efficient data transfer
- Lower power consumption than software SHA

### Bitcoin Mining Context

Bitcoin uses SHA-256d (double SHA-256) for mining:
1. First SHA-256 of block header
2. Second SHA-256 of the first result

Both operations benefit from hardware acceleration, resulting in significant hashrate improvement.

## Thermal Considerations

⚠️ **Important**: Hardware SHA acceleration increases the computational load and may raise chip temperature.

### Recommendations

- **Passive cooling**: Sufficient for single-core mining at ≤160 MHz
- **Active cooling (fan)**: **REQUIRED** for:
  - Dual-core mining at any clock speed
  - Single-core mining at 240 MHz
  - Extended mining sessions
  - Mining arrays (4+ devices)

Monitor temperature during mining:
- **Safe**: < 70°C
- **Warning**: 70-80°C
- **Critical**: > 85°C

See [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md) for detailed thermal management guidance.

## Benchmarking

To measure performance improvement:

1. Build with hardware acceleration (default):
   ```bash
   idf.py build flash monitor
   ```

2. Note the hashrate from serial output after ~30 seconds

3. To test software-only (for comparison):
   - Edit `sdkconfig.defaults`
   - Change `CONFIG_MBEDTLS_HARDWARE_SHA=y` to `CONFIG_MBEDTLS_HARDWARE_SHA=n`
   - Rebuild and reflash

Compare hashrates to see the improvement from hardware acceleration.

## Compatibility

- **Chip**: ESP32-S3 (all variants)
- **ESP-IDF**: v5.1.2 or later
- **mbedTLS**: Included in ESP-IDF

Hardware SHA acceleration is specific to ESP32-S3. Other ESP32 variants (ESP32, ESP32-C3, ESP32-S2) have different or no SHA acceleration capabilities.

## References

- [ESP32-S3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf) - Chapter on SHA Accelerator
- [ESP-IDF mbedTLS Component](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/protocols/mbedtls.html)
- [Bitcoin SHA-256d Hashing](https://en.bitcoin.it/wiki/Hash)

## Future Optimizations

Potential further improvements:
- Direct SHA peripheral API usage (bypass mbedTLS overhead)
- Midstate pre-computation for fixed header bytes
- Assembly optimizations for non-SHA operations
- DMA for data transfer to SHA peripheral

## Performance Testing & Verification

### Why Hardware SHA Matters (Even with Optimized Allocation)

The develop branch optimized heap allocation patterns, which is excellent! However, hardware SHA acceleration is **complementary** and addresses a different bottleneck:

- **Heap optimization** (develop): Removes malloc/free overhead (~10-20% improvement)
- **Hardware SHA** (this PR): Accelerates the actual SHA-256 computation (~2-4x improvement)

These optimizations stack multiplicatively, not additively.

### Verifying Hardware SHA is Active

1. **Check Boot Log**: Look for this message at startup:
   ```
   I (xxx) BTC_MINER: Hardware SHA acceleration: ENABLED
   ```

2. **Build Configuration**: Run `idf.py menuconfig` and verify:
   - Navigate to: Component config → mbedTLS → Hardware acceleration
   - Verify "Hardware SHA acceleration" is checked

3. **Performance Test**: 
   ```bash
   # Build with hardware SHA (default)
   idf.py build flash monitor
   # Note hashrate after 60 seconds
   
   # Build without hardware SHA
   # Edit sdkconfig.defaults: CONFIG_MBEDTLS_HARDWARE_SHA=n
   idf.py fullclean build flash monitor
   # Compare hashrate
   ```

Expected: Measurable difference even with optimized allocations.

### Understanding the Performance Stack

```
Total Hashrate = (Base SHA Speed) × (1 - Allocation Overhead) × (1 - Other Overhead)

Without optimizations:
  ~10 kH/s = (Slow Software SHA) × (0.8 due to malloc/free) × (0.9 other)

With heap optimization only (develop):
  ~20 kH/s = (Slow Software SHA) × (1.0 no malloc) × (0.9 other)

With hardware SHA only (old PR):
  ~25 kH/s = (Fast Hardware SHA) × (0.8 due to malloc/free) × (0.9 other)

With both optimizations (this PR):
  ~40-60 kH/s = (Fast Hardware SHA) × (1.0 no malloc) × (0.9 other)
```

### If No Performance Difference is Observed

If hardware SHA shows no improvement over develop's optimized software implementation, possible causes:

1. **Hardware SHA not actually enabled**: Check sdkconfig after build with `grep MBEDTLS_HARDWARE_SHA sdkconfig`
2. **ESP-IDF version issue**: Hardware SHA support requires ESP-IDF 4.4+
3. **mbedTLS not using hardware**: Check mbedTLS component configuration
4. **Bottleneck elsewhere**: If CPU is waiting on display/WiFi, SHA speed won't matter

### Debugging Hardware SHA

To verify mbedTLS is using hardware:
```c
// Add to app_main() temporarily:
#ifdef CONFIG_MBEDTLS_HARDWARE_SHA
    ESP_LOGI(TAG, "MBEDTLS_HARDWARE_SHA defined");
#else
    ESP_LOGI(TAG, "MBEDTLS_HARDWARE_SHA NOT defined");
#endif
```

If the macro is not defined, hardware SHA is not active, and the config needs adjustment.
