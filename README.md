Project to test a setup for ESP32 S3 N16R8 as a bitcoin solo miner or ticket miner.

IDE: VSCode, using the ESP-IDF: Explorer extension.

Project first auto build for ESP32, then changed to ESP32S3 (via ESP-PROG).

## WiFi Configuration

To configure your WiFi credentials:

1. Copy the example configuration file:
   ```bash
   cp main/config.h.example main/config.h
   ```

2. Edit `main/config.h` and update the WiFi credentials:
   ```c
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASS "your_wifi_password"
   ```

3. The `config.h` file is gitignored to prevent accidentally committing your credentials.

Note: Never commit your `main/config.h` file with real credentials to version control.

## Performance Benchmarking

This project includes a comprehensive performance benchmarking system to identify optimization opportunities and track improvements.

### Features

- Automated GitHub Actions workflow for performance analysis
- Reusable benchmark component for ESP32
- Identification of performance-critical functions
- Statistical analysis (min, max, average execution times)

### Key Functions Monitored

1. **double_sha256()** - Bitcoin double SHA-256 hashing
2. **count_leading_zeros()** - Difficulty calculation
3. **mining_task()** - Main mining loop

For detailed information, see [BENCHMARKING.md](BENCHMARKING.md)

### Quick Start

To use benchmarking in your code:

```c
#include "benchmark.h"

benchmark_init();

BENCHMARK("my_function", {
    my_function();
});

benchmark_print_all();
```

See `examples/benchmark_integration_example.c` for more examples.

## Testing

This project includes comprehensive unit tests for all components.

### Running Tests

To build and run the unit tests:

```bash
cd test
idf.py build
idf.py flash monitor
```

For more details, see [test/README.md](test/README.md)

### Test Coverage

- **Benchmark Component**: 12 test cases covering initialization, timing, statistics, and edge cases
- Tests are automatically built in the CI/CD pipeline

### Continuous Integration

Tests are automatically executed in the GitHub Actions workflow on every push and pull request.
