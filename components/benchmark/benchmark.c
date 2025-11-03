#include "benchmark.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>
#include <math.h>
#include <inttypes.h>

#define MAX_BENCHMARKS 16
#define TAG "BENCHMARK"

typedef struct {
    char name[32];
    uint64_t total_iterations;
    uint64_t total_time_us;
    uint64_t min_time_us;
    uint64_t max_time_us;
    bool active;
} benchmark_data_t;

static benchmark_data_t benchmarks[MAX_BENCHMARKS];
static int num_benchmarks = 0;

void benchmark_init(void)
{
    memset(benchmarks, 0, sizeof(benchmarks));
    num_benchmarks = 0;
    ESP_LOGI(TAG, "Benchmark system initialized");
}

static benchmark_data_t* find_or_create_benchmark(const char *name)
{
    // First, try to find existing benchmark
    for (int i = 0; i < num_benchmarks; i++) {
        if (strcmp(benchmarks[i].name, name) == 0) {
            return &benchmarks[i];
        }
    }
    
    // Create new benchmark if space available
    if (num_benchmarks < MAX_BENCHMARKS) {
        benchmark_data_t *bench = &benchmarks[num_benchmarks];
        strncpy(bench->name, name, sizeof(bench->name) - 1);
        bench->name[sizeof(bench->name) - 1] = '\0';
        bench->total_iterations = 0;
        bench->total_time_us = 0;
        bench->min_time_us = UINT64_MAX;
        bench->max_time_us = 0;
        bench->active = true;
        num_benchmarks++;
        return bench;
    }
    
    ESP_LOGW(TAG, "Maximum number of benchmarks reached");
    return NULL;
}

uint64_t benchmark_start(const char *name)
{
    return esp_timer_get_time();
}

void benchmark_end(const char *name, uint64_t start_time)
{
    uint64_t end_time = esp_timer_get_time();
    uint64_t elapsed = end_time - start_time;
    
    benchmark_data_t *bench = find_or_create_benchmark(name);
    if (bench == NULL) {
        return;
    }
    
    bench->total_iterations++;
    bench->total_time_us += elapsed;
    
    if (elapsed < bench->min_time_us) {
        bench->min_time_us = elapsed;
    }
    
    if (elapsed > bench->max_time_us) {
        bench->max_time_us = elapsed;
    }
}

bool benchmark_get_stats(const char *name, benchmark_stats_t *stats)
{
    if (stats == NULL) {
        return false;
    }
    
    for (int i = 0; i < num_benchmarks; i++) {
        if (strcmp(benchmarks[i].name, name) == 0) {
            stats->name = benchmarks[i].name;
            stats->total_iterations = benchmarks[i].total_iterations;
            stats->total_time_us = benchmarks[i].total_time_us;
            stats->min_time_us = benchmarks[i].min_time_us;
            stats->max_time_us = benchmarks[i].max_time_us;
            
            if (stats->total_iterations > 0) {
                stats->avg_time_us = (double)stats->total_time_us / stats->total_iterations;
            } else {
                stats->avg_time_us = 0.0;
            }
            
            return true;
        }
    }
    
    return false;
}

void benchmark_print_all(void)
{
    ESP_LOGI(TAG, "=== Performance Benchmark Results ===");
    ESP_LOGI(TAG, "%-20s %10s %12s %12s %12s %12s", 
             "Name", "Iterations", "Total(us)", "Min(us)", "Max(us)", "Avg(us)");
    ESP_LOGI(TAG, "--------------------------------------------------------------------------------");
    
    for (int i = 0; i < num_benchmarks; i++) {
        if (benchmarks[i].active && benchmarks[i].total_iterations > 0) {
            double avg = (double)benchmarks[i].total_time_us / benchmarks[i].total_iterations;
            
            ESP_LOGI(TAG, "%-20s %10" PRIu64 " %12" PRIu64 " %12" PRIu64 " %12" PRIu64 " %12.2f",
                     benchmarks[i].name,
                     benchmarks[i].total_iterations,
                     benchmarks[i].total_time_us,
                     benchmarks[i].min_time_us,
                     benchmarks[i].max_time_us,
                     avg);
            
            // Calculate performance metrics
            if (avg > 0) {
                double ops_per_sec = 1000000.0 / avg;
                ESP_LOGI(TAG, "  -> %.2f ops/sec (%.2f ms per op)", 
                         ops_per_sec, avg / 1000.0);
            }
        }
    }
    
    ESP_LOGI(TAG, "================================================================================");
}

void benchmark_reset_all(void)
{
    for (int i = 0; i < num_benchmarks; i++) {
        benchmarks[i].total_iterations = 0;
        benchmarks[i].total_time_us = 0;
        benchmarks[i].min_time_us = UINT64_MAX;
        benchmarks[i].max_time_us = 0;
    }
    
    ESP_LOGI(TAG, "All benchmarks reset");
}
