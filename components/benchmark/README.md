# Benchmark Component

This component provides performance benchmarking utilities for ESP32 applications.

## Features

- Lightweight performance measurement using ESP32 high-resolution timer
- Track multiple benchmarks simultaneously
- Automatic calculation of min, max, and average execution times
- Easy-to-use macro for benchmarking code blocks

## Usage

### Basic Usage

```c
#include "benchmark.h"

void app_main(void)
{
    // Initialize benchmark system
    benchmark_init();
    
    // Benchmark a function
    uint64_t start = benchmark_start("my_function");
    my_function();
    benchmark_end("my_function", start);
    
    // Print all benchmark results
    benchmark_print_all();
}
```

### Using the BENCHMARK Macro

```c
#include "benchmark.h"

void app_main(void)
{
    benchmark_init();
    
    // Benchmark a code block
    BENCHMARK("hash_calculation", {
        double_sha256(data, len, hash);
    });
    
    benchmark_print_all();
}
```

### Integration with Mining Functions

To benchmark the critical mining functions:

```c
// In your mining loop
benchmark_init();

for (int i = 0; i < 1000; i++) {
    BENCHMARK("double_sha256", {
        double_sha256(block_header, 80, hash);
    });
    
    BENCHMARK("count_leading_zeros", {
        difficulty = count_leading_zeros(hash);
    });
}

benchmark_print_all();
```

## Output Example

```
I (12345) BENCHMARK: === Performance Benchmark Results ===
I (12346) BENCHMARK: Name                 Iterations   Total(us)     Min(us)     Max(us)     Avg(us)
I (12347) BENCHMARK: --------------------------------------------------------------------------------
I (12348) BENCHMARK: double_sha256              1000     1234567        1200        1350     1234.57
I (12349) BENCHMARK:   -> 810.00 ops/sec (1.23 ms per op)
I (12350) BENCHMARK: count_leading_zeros        1000       12345          10          15       12.35
I (12351) BENCHMARK:   -> 80972.00 ops/sec (0.01 ms per op)
```

## API Reference

### Functions

- `benchmark_init()` - Initialize the benchmark system
- `benchmark_start(name)` - Start timing a benchmark
- `benchmark_end(name, start_time)` - End timing a benchmark
- `benchmark_get_stats(name, stats)` - Get statistics for a specific benchmark
- `benchmark_print_all()` - Print all benchmark results
- `benchmark_reset_all()` - Reset all benchmark statistics

### Macro

- `BENCHMARK(name, code)` - Benchmark a code block

## Performance Considerations

- The benchmark system uses the ESP32 high-resolution timer (`esp_timer_get_time()`)
- Minimal overhead: typically < 1μs per measurement
- Maximum 16 concurrent benchmarks can be tracked
- Thread-safe for measurements from multiple tasks
