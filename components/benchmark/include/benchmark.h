#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Benchmark statistics structure
 */
typedef struct {
    const char *name;           // Name of the benchmark
    uint64_t total_iterations;  // Total number of iterations
    uint64_t total_time_us;     // Total time in microseconds
    uint64_t min_time_us;       // Minimum iteration time
    uint64_t max_time_us;       // Maximum iteration time
    double avg_time_us;         // Average iteration time
} benchmark_stats_t;

/**
 * @brief Initialize benchmarking system
 */
void benchmark_init(void);

/**
 * @brief Start a benchmark measurement
 * @param name Name of the benchmark
 * @return Benchmark handle (timestamp)
 */
uint64_t benchmark_start(const char *name);

/**
 * @brief End a benchmark measurement
 * @param name Name of the benchmark
 * @param start_time Start timestamp from benchmark_start
 */
void benchmark_end(const char *name, uint64_t start_time);

/**
 * @brief Get statistics for a specific benchmark
 * @param name Name of the benchmark
 * @param stats Pointer to stats structure to fill
 * @return true if benchmark found, false otherwise
 */
bool benchmark_get_stats(const char *name, benchmark_stats_t *stats);

/**
 * @brief Print all benchmark statistics
 */
void benchmark_print_all(void);

/**
 * @brief Reset all benchmark statistics
 */
void benchmark_reset_all(void);

/**
 * @brief Macro to benchmark a code block
 */
#define BENCHMARK(name, code) \
    do { \
        uint64_t _start = benchmark_start(name); \
        code; \
        benchmark_end(name, _start); \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // BENCHMARK_H
