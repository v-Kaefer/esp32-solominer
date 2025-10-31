#include <string.h>
#include "unity.h"
#include "ssd1306.h"

// Mock I2C port for testing
#define TEST_I2C_PORT I2C_NUM_0

// Test device structure
static SSD1306_t test_dev;

void setUp(void)
{
    memset(&test_dev, 0, sizeof(SSD1306_t));
}

void tearDown(void)
{
    // Cleanup after each test
}

// Test device initialization
void test_ssd1306_init_structure(void)
{
    i2c_master_init_ssd1306(&test_dev, TEST_I2C_PORT, 128, 64, 0x3C);
    
    TEST_ASSERT_EQUAL(TEST_I2C_PORT, test_dev.i2c_port);
    TEST_ASSERT_EQUAL(0x3C, test_dev.i2c_addr);
    TEST_ASSERT_EQUAL(128, test_dev.width);
    TEST_ASSERT_EQUAL(64, test_dev.height);
    TEST_ASSERT_EQUAL(8, test_dev.pages);
}

// Test device initialization with different dimensions
void test_ssd1306_init_different_size(void)
{
    i2c_master_init_ssd1306(&test_dev, TEST_I2C_PORT, 128, 32, 0x3D);
    
    TEST_ASSERT_EQUAL(128, test_dev.width);
    TEST_ASSERT_EQUAL(32, test_dev.height);
    TEST_ASSERT_EQUAL(4, test_dev.pages);
    TEST_ASSERT_EQUAL(0x3D, test_dev.i2c_addr);
}

// Test pages calculation
void test_ssd1306_pages_calculation(void)
{
    // 64 pixels height = 8 pages
    i2c_master_init_ssd1306(&test_dev, TEST_I2C_PORT, 128, 64, 0x3C);
    TEST_ASSERT_EQUAL(8, test_dev.pages);
    
    // 32 pixels height = 4 pages
    i2c_master_init_ssd1306(&test_dev, TEST_I2C_PORT, 128, 32, 0x3C);
    TEST_ASSERT_EQUAL(4, test_dev.pages);
    
    // 16 pixels height = 2 pages
    i2c_master_init_ssd1306(&test_dev, TEST_I2C_PORT, 128, 16, 0x3C);
    TEST_ASSERT_EQUAL(2, test_dev.pages);
}

// Test that device structure is properly initialized
void test_ssd1306_device_not_null(void)
{
    SSD1306_t dev;
    memset(&dev, 0xFF, sizeof(SSD1306_t)); // Fill with garbage
    
    i2c_master_init_ssd1306(&dev, TEST_I2C_PORT, 128, 64, 0x3C);
    
    // Verify the structure has been properly initialized
    TEST_ASSERT_NOT_EQUAL(0xFF, dev.width);
    TEST_ASSERT_NOT_EQUAL(0xFF, dev.height);
    TEST_ASSERT_EQUAL(128, dev.width);
    TEST_ASSERT_EQUAL(64, dev.height);
}

// Register tests with Unity
void test_ssd1306_functions(void)
{
    RUN_TEST(test_ssd1306_init_structure);
    RUN_TEST(test_ssd1306_init_different_size);
    RUN_TEST(test_ssd1306_pages_calculation);
    RUN_TEST(test_ssd1306_device_not_null);
}
