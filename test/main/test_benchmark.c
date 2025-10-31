#include <string.h>
#include <inttypes.h>
#include "unity.h"
#include "benchmark.h"
#include "esp_timer.h"

// Test fixture setup and teardown
void setUp(void)
{
    benchmark_init();
}

void tearDown(void)
{
    benchmark_reset_all();
}

// Test: Benchmark initialization
TEST_CASE("Benchmark initialization", "[benchmark]")
{
    benchmark_init();
    
    // After init, getting stats for non-existent benchmark should return false
    benchmark_stats_t stats;
    TEST_ASSERT_FALSE(benchmark_get_stats("non_existent", &stats));
}

// Test: Basic benchmark start and end
TEST_CASE("Basic benchmark timing", "[benchmark]")
{
    const char *test_name = "test_function";
    
    uint64_t start = benchmark_start(test_name);
    
    // Simulate some work (busy wait for ~1ms)
    uint64_t work_start = esp_timer_get_time();
    while ((esp_timer_get_time() - work_start) < 1000) {
        // Busy wait
    }
    
    benchmark_end(test_name, start);
    
    // Verify stats were recorded
    benchmark_stats_t stats;
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(1, stats.total_iterations);
    TEST_ASSERT_GREATER_THAN(0, stats.total_time_us);
    TEST_ASSERT_GREATER_OR_EQUAL(900, stats.min_time_us); // Allow some tolerance
}

// Test: Multiple iterations
TEST_CASE("Multiple benchmark iterations", "[benchmark]")
{
    const char *test_name = "multi_test";
    const int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        uint64_t start = benchmark_start(test_name);
        
        // Small delay
        uint64_t work_start = esp_timer_get_time();
        while ((esp_timer_get_time() - work_start) < 10) {
            // Busy wait ~10us
        }
        
        benchmark_end(test_name, start);
    }
    
    benchmark_stats_t stats;
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(iterations, stats.total_iterations);
    TEST_ASSERT_GREATER_THAN(0, stats.avg_time_us);
    TEST_ASSERT_LESS_OR_EQUAL(stats.max_time_us, stats.total_time_us);
    TEST_ASSERT_GREATER_OR_EQUAL(stats.min_time_us, stats.avg_time_us / 2);
}

// Test: BENCHMARK macro
TEST_CASE("BENCHMARK macro", "[benchmark]")
{
    const char *test_name = "macro_test";
    
    BENCHMARK(test_name, {
        // Simulate work
        volatile int sum = 0;
        for (int i = 0; i < 1000; i++) {
            sum += i;
        }
    });
    
    benchmark_stats_t stats;
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(1, stats.total_iterations);
    TEST_ASSERT_GREATER_THAN(0, stats.total_time_us);
}

// Test: Min/Max tracking
TEST_CASE("Min and max time tracking", "[benchmark]")
{
    const char *test_name = "minmax_test";
    
    // First iteration - short
    uint64_t start1 = benchmark_start(test_name);
    uint64_t work_start = esp_timer_get_time();
    while ((esp_timer_get_time() - work_start) < 100) {} // ~100us
    benchmark_end(test_name, start1);
    
    // Second iteration - longer
    uint64_t start2 = benchmark_start(test_name);
    work_start = esp_timer_get_time();
    while ((esp_timer_get_time() - work_start) < 500) {} // ~500us
    benchmark_end(test_name, start2);
    
    // Third iteration - medium
    uint64_t start3 = benchmark_start(test_name);
    work_start = esp_timer_get_time();
    while ((esp_timer_get_time() - work_start) < 300) {} // ~300us
    benchmark_end(test_name, start3);
    
    benchmark_stats_t stats;
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(3, stats.total_iterations);
    TEST_ASSERT_LESS_THAN(stats.max_time_us, stats.min_time_us * 6); // Max should be > Min
    TEST_ASSERT_GREATER_THAN(stats.min_time_us, 50); // At least 50us
}

// Test: Multiple benchmarks
TEST_CASE("Multiple concurrent benchmarks", "[benchmark]")
{
    const char *test1 = "benchmark_1";
    const char *test2 = "benchmark_2";
    const char *test3 = "benchmark_3";
    
    // Run different benchmarks
    BENCHMARK(test1, {
        volatile int x = 0;
        for (int i = 0; i < 100; i++) x += i;
    });
    
    BENCHMARK(test2, {
        volatile int x = 0;
        for (int i = 0; i < 200; i++) x += i;
    });
    
    BENCHMARK(test3, {
        volatile int x = 0;
        for (int i = 0; i < 300; i++) x += i;
    });
    
    // Verify all three are tracked
    benchmark_stats_t stats1, stats2, stats3;
    TEST_ASSERT_TRUE(benchmark_get_stats(test1, &stats1));
    TEST_ASSERT_TRUE(benchmark_get_stats(test2, &stats2));
    TEST_ASSERT_TRUE(benchmark_get_stats(test3, &stats3));
    
    TEST_ASSERT_EQUAL(1, stats1.total_iterations);
    TEST_ASSERT_EQUAL(1, stats2.total_iterations);
    TEST_ASSERT_EQUAL(1, stats3.total_iterations);
}

// Test: Reset functionality
TEST_CASE("Benchmark reset", "[benchmark]")
{
    const char *test_name = "reset_test";
    
    // Run benchmark
    BENCHMARK(test_name, {
        volatile int x = 0;
        for (int i = 0; i < 100; i++) x += i;
    });
    
    benchmark_stats_t stats;
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(1, stats.total_iterations);
    
    // Reset all
    benchmark_reset_all();
    
    // Stats should be reset but benchmark should still exist
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(0, stats.total_iterations);
    TEST_ASSERT_EQUAL(0, stats.total_time_us);
}

// Test: Get stats with NULL pointer
TEST_CASE("Get stats with NULL pointer", "[benchmark]")
{
    const char *test_name = "null_test";
    
    BENCHMARK(test_name, {
        volatile int x = 42;
    });
    
    // Should return false with NULL stats pointer
    TEST_ASSERT_FALSE(benchmark_get_stats(test_name, NULL));
}

// Test: Non-existent benchmark
TEST_CASE("Get stats for non-existent benchmark", "[benchmark]")
{
    benchmark_stats_t stats;
    TEST_ASSERT_FALSE(benchmark_get_stats("this_does_not_exist", &stats));
}

// Test: Average calculation
TEST_CASE("Average time calculation", "[benchmark]")
{
    const char *test_name = "average_test";
    const int iterations = 10;
    
    for (int i = 0; i < iterations; i++) {
        uint64_t start = benchmark_start(test_name);
        
        // Consistent work
        uint64_t work_start = esp_timer_get_time();
        while ((esp_timer_get_time() - work_start) < 100) {} // ~100us each
        
        benchmark_end(test_name, start);
    }
    
    benchmark_stats_t stats;
    TEST_ASSERT_TRUE(benchmark_get_stats(test_name, &stats));
    TEST_ASSERT_EQUAL(iterations, stats.total_iterations);
    
    // Average should be roughly total / iterations
    double calculated_avg = (double)stats.total_time_us / stats.total_iterations;
    TEST_ASSERT_DOUBLE_WITHIN(1.0, calculated_avg, stats.avg_time_us);
}
