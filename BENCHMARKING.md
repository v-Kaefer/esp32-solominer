# Performance Benchmarking Workflow

This document describes the performance benchmarking workflow for the ESP32 Bitcoin Solo Miner project.

## Overview

The project now includes a comprehensive performance benchmarking system to identify optimization opportunities and track performance improvements over time.

## Components

### 1. GitHub Actions Workflow

The `.github/workflows/performance-benchmark.yml` workflow automatically:
- Builds the project for ESP32-S3 target
- Analyzes build sizes
- Generates performance reports
- Identifies key functions for optimization

**Trigger conditions:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop` branches
- Manual workflow dispatch

### 2. Benchmark Component

A reusable ESP32 component (`components/benchmark/`) provides:
- High-precision timing utilities
- Statistical analysis (min, max, average)
- Easy-to-use macros for code instrumentation
- Low overhead measurement

## Identified Performance-Critical Functions

The following functions have been identified as candidates for optimization:

### 1. `double_sha256()`
**Purpose**: Performs double SHA-256 hashing for Bitcoin mining
**Current Implementation**: Uses mbedtls library
**Optimization Opportunities**:
- Enable ESP32-S3 hardware SHA acceleration
- Optimize context reuse
- Consider assembly optimization for critical paths

### 2. `count_leading_zeros()`
**Purpose**: Calculates hash difficulty by counting leading zero bits
**Current Implementation**: Byte-by-byte iteration
**Optimization Opportunities**:
- Use CLZ (Count Leading Zeros) CPU instruction
- Optimize loop unrolling
- Consider lookup table approach

### 3. `mining_task()`
**Purpose**: Main mining loop
**Current Implementation**: Single-threaded execution
**Optimization Opportunities**:
- Utilize both cores of ESP32-S3
- Implement work distribution
- Optimize nonce incrementing

## How to Use the Benchmark Component

### Integration Example

To benchmark your functions, include the benchmark component:

```c
#include "benchmark.h"

void app_main(void)
{
    benchmark_init();
    
    // Run mining with benchmarks
    for (int i = 0; i < 10000; i++) {
        BENCHMARK("double_sha256", {
            double_sha256(block_header, 80, hash);
        });
        
        BENCHMARK("count_leading_zeros", {
            difficulty = count_leading_zeros(hash);
        });
    }
    
    // Print results
    benchmark_print_all();
}
```

### Expected Output

```
I (12345) BENCHMARK: === Performance Benchmark Results ===
I (12346) BENCHMARK: Name                 Iterations   Total(us)     Min(us)     Max(us)     Avg(us)
I (12347) BENCHMARK: --------------------------------------------------------------------------------
I (12348) BENCHMARK: double_sha256             10000    12345678        1200        1350     1234.57
I (12349) BENCHMARK:   -> 810.00 ops/sec (1.23 ms per op)
I (12350) BENCHMARK: count_leading_zeros       10000      123456          10          15       12.35
I (12351) BENCHMARK:   -> 80972.00 ops/sec (0.01 ms per op)
```

## Optimization Recommendations

### Priority 1: Hardware SHA Acceleration
Enable ESP32-S3's built-in SHA hardware acceleration:
```c
// In sdkconfig or menuconfig
CONFIG_MBEDTLS_HARDWARE_SHA=y
```

### Priority 2: Dual-Core Mining
Implement dual-core mining to theoretically double hashrate:
```c
xTaskCreatePinnedToCore(mining_task, "mining_task_0", 4096, NULL, 5, NULL, 0);
xTaskCreatePinnedToCore(mining_task, "mining_task_1", 4096, NULL, 5, NULL, 1);
```

### Priority 3: Function Optimizations
- Use `__builtin_clz()` for count_leading_zeros
- Reduce memory allocations in hot paths
- Profile with real hardware measurements

## Testing Improvements

After implementing optimizations:
1. Run benchmarks before and after
2. Compare results using the workflow artifacts
3. Ensure hashrate improvements are measurable
4. Verify correctness of results

## Continuous Monitoring

The GitHub Actions workflow runs automatically on:
- Every push to main/develop branches
- Every pull request
- Manual trigger for ad-hoc testing

Benchmark reports are uploaded as artifacts and can be downloaded for detailed analysis.

## Future Enhancements

Potential improvements to the benchmarking system:
1. Hardware-specific benchmarks (JTAG profiling)
2. Power consumption measurements
3. Temperature monitoring during stress tests
4. Comparative analysis across commits
5. Automated performance regression detection
