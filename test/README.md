# ESP32 Bitcoin Miner - Unit Tests

This directory contains unit tests for the ESP32 Bitcoin Miner project.

## Running Tests

### Build Tests

To build the test application:

```bash
cd test
idf.py build
```

### Flash and Run Tests

To flash the test firmware to your ESP32-S3 and run the tests:

```bash
cd test
idf.py flash monitor
```

### Run Specific Test

Once the test app is running, you can use the test menu to run specific tests:

```
Press ENTER to see the list of tests.
Enter test for running.
```

## Test Structure

- `test/main/test_main.c` - Main test application entry point
- `test/main/test_benchmark.c` - Unit tests for the benchmark component

## Test Coverage

### Benchmark Component Tests

The benchmark component has comprehensive test coverage including:

1. **Initialization Tests**
   - Verifies proper initialization of the benchmark system
   - Tests behavior with non-existent benchmarks

2. **Basic Timing Tests**
   - Tests single benchmark measurement
   - Verifies timing accuracy

3. **Multiple Iterations Tests**
   - Tests multiple iterations of the same benchmark
   - Verifies statistical calculations

4. **BENCHMARK Macro Tests**
   - Tests the convenience macro for benchmarking code blocks

5. **Min/Max Tracking Tests**
   - Verifies correct tracking of minimum and maximum times

6. **Concurrent Benchmarks Tests**
   - Tests multiple simultaneous benchmarks
   - Verifies independence of different benchmarks

7. **Reset Functionality Tests**
   - Tests benchmark reset functionality
   - Verifies proper cleanup

8. **Edge Cases Tests**
   - Tests with NULL pointers
   - Tests non-existent benchmarks
   - Verifies error handling

9. **Statistical Accuracy Tests**
   - Tests average calculation
   - Verifies statistical accuracy

## Adding New Tests

To add new tests:

1. Create a new test file in `test/main/` (e.g., `test_myfeature.c`)
2. Include the Unity test framework: `#include "unity.h"`
3. Write your test cases using Unity macros:
   ```c
   TEST_CASE("My test description", "[tag]")
   {
       TEST_ASSERT_EQUAL(expected, actual);
   }
   ```
4. Update `test/main/CMakeLists.txt` to include your new test file in `SRCS`

## Unity Test Framework

This project uses the Unity test framework provided by ESP-IDF. For more information on Unity assertions and test features, see:
- [Unity Assertions Reference](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md)
- [ESP-IDF Unit Testing Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)

## Continuous Integration

The unit tests are automatically built as part of the GitHub Actions workflow in `.github/workflows/performance-benchmark.yml`.

## Test Results

Test results can be viewed in:
- GitHub Actions workflow logs
- Serial monitor when running on hardware
- Uploaded test artifacts in GitHub Actions
