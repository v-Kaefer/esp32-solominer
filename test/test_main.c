#include <stdio.h>
#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

// Unity test framework setup
void setUp(void)
{
    // Setup code before each test
}

void tearDown(void)
{
    // Cleanup code after each test
}

void app_main(void)
{
    printf("\n");
    printf("==================================\n");
    printf("ESP32 Bitcoin Miner - Unit Tests\n");
    printf("==================================\n");
    
    UNITY_BEGIN();
    
    // Test functions will be registered here
    unity_run_tests_by_tag("[mining]", false);
    unity_run_tests_by_tag("[ssd1306]", false);
    
    UNITY_END();
    
    // For ESP-IDF, wait a bit before finishing
    vTaskDelay(pdMS_TO_TICKS(1000));
}
