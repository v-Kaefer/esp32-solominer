/*
 * Example: How to integrate benchmarking into the mining code
 * 
 * This example shows how to use the benchmark component to measure
 * performance of key mining functions.
 * 
 * Note: This is an example file demonstrating usage. In a real implementation,
 * these function declarations would be in a proper header file.
 */

#include "benchmark.h"
#include <string.h>
#include <inttypes.h>

// These functions would typically be declared in a header file (e.g., mining.h)
// For this example, we declare them here to show the usage pattern
extern void double_sha256(const uint8_t* data, size_t len, uint8_t* hash);
extern uint32_t count_leading_zeros(const uint8_t* hash);

void example_benchmark_mining_functions(void)
{
    // Initialize benchmark system
    benchmark_init();
    
    // Prepare test data
    uint8_t block_header[80];
    uint8_t hash[32];
    memset(block_header, 0x55, 80);
    
    // Benchmark double_sha256 - most critical function
    printf("\nBenchmarking double_sha256() - 10000 iterations\n");
    for (int i = 0; i < 10000; i++) {
        BENCHMARK("double_sha256", {
            double_sha256(block_header, 80, hash);
        });
        
        // Vary the nonce to get different results
        block_header[76] = i & 0xFF;
        block_header[77] = (i >> 8) & 0xFF;
    }
    
    // Benchmark count_leading_zeros
    printf("\nBenchmarking count_leading_zeros() - 10000 iterations\n");
    for (int i = 0; i < 10000; i++) {
        BENCHMARK("count_leading_zeros", {
            uint32_t difficulty = count_leading_zeros(hash);
            (void)difficulty; // Suppress unused variable warning
        });
    }
    
    // Print comprehensive results
    printf("\n");
    benchmark_print_all();
    
    // Get specific stats
    benchmark_stats_t stats;
    if (benchmark_get_stats("double_sha256", &stats)) {
        printf("\nDetailed stats for double_sha256:\n");
        printf("  Iterations: %" PRIu64 "\n", stats.total_iterations);
        printf("  Average time: %.2f us\n", stats.avg_time_us);
        printf("  Estimated hashrate: %.2f H/s\n", 1000000.0 / stats.avg_time_us);
    }
}

void example_benchmark_with_manual_timing(void)
{
    benchmark_init();
    
    uint8_t block_header[80];
    uint8_t hash[32];
    memset(block_header, 0xAA, 80);
    
    // Manual benchmark timing
    for (int i = 0; i < 1000; i++) {
        uint64_t start = benchmark_start("manual_hash");
        
        // Your code here
        double_sha256(block_header, 80, hash);
        
        benchmark_end("manual_hash", start);
    }
    
    benchmark_print_all();
}

void example_benchmark_optimization_comparison(void)
{
    // This example shows how to compare before/after optimization
    benchmark_init();
    
    uint8_t data[80];
    uint8_t hash[32];
    memset(data, 0, 80);
    
    printf("Testing current implementation...\n");
    for (int i = 0; i < 5000; i++) {
        BENCHMARK("current_implementation", {
            double_sha256(data, 80, hash);
        });
    }
    
    // After implementing optimization, add:
    // for (int i = 0; i < 5000; i++) {
    //     BENCHMARK("optimized_implementation", {
    //         double_sha256_optimized(data, 80, hash);
    //     });
    // }
    
    benchmark_print_all();
    
    // Compare results
    benchmark_stats_t current;
    if (benchmark_get_stats("current_implementation", &current)) {
        printf("\nCurrent implementation: %.2f us per operation\n", current.avg_time_us);
        printf("Theoretical max hashrate: %.2f H/s\n", 1000000.0 / current.avg_time_us);
    }
}

/*
 * Integration into main.c mining_task:
 * 
 * void mining_task(void *pvParameters)
 * {
 *     // Initialize at the start
 *     benchmark_init();
 *     
 *     uint8_t hash[32];
 *     uint64_t hash_count = 0;
 *     
 *     // ... existing initialization code ...
 *     
 *     while(1) {
 *         // Benchmark the hash calculation
 *         BENCHMARK("double_sha256", {
 *             double_sha256(block_header, 80, hash);
 *         });
 *         
 *         hash_count++;
 *         total_hashes++;
 *         
 *         // Benchmark difficulty calculation
 *         uint32_t difficulty;
 *         BENCHMARK("count_leading_zeros", {
 *             difficulty = count_leading_zeros(hash);
 *         });
 *         
 *         // ... rest of mining code ...
 *         
 *         // Print benchmark results every N iterations
 *         if (hash_count % 10000 == 0) {
 *             benchmark_print_all();
 *             benchmark_reset_all(); // Reset for next measurement window
 *         }
 *     }
 * }
 */
