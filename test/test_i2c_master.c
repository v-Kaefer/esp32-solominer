#include <string.h>
#include "unity.h"
#include "driver/i2c_master.h"

// Test I2C master configuration initialization
void test_i2c_master_config_default(void)
{
    i2c_master_config_t config = I2C_MASTER_DEFAULT_CONFIG();
    
    TEST_ASSERT_EQUAL(I2C_NUM_0, config.i2c_port);
    TEST_ASSERT_EQUAL(GPIO_NUM_15, config.sda_io_num);
    TEST_ASSERT_EQUAL(GPIO_NUM_9, config.scl_io_num);
    TEST_ASSERT_EQUAL(100000, config.clk_speed);
    TEST_ASSERT_TRUE(config.sda_pullup_en);
    TEST_ASSERT_TRUE(config.scl_pullup_en);
    TEST_ASSERT_EQUAL(1000, config.timeout_ms);
}

// Test display configuration initialization
void test_display_config_default(void)
{
    display_config_t config = DISPLAY_DEFAULT_CONFIG();
    
    TEST_ASSERT_EQUAL(DISPLAY_DRIVER_SSD1306, config.driver_ic);
    TEST_ASSERT_EQUAL(0x3C, config.i2c_addr);
    TEST_ASSERT_EQUAL(128, config.width);
    TEST_ASSERT_EQUAL(64, config.height);
    TEST_ASSERT_EQUAL(0xCF, config.contrast);
    TEST_ASSERT_TRUE(config.low_power_mode);
}

// Test voltage validation
void test_voltage_validation_valid(void)
{
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(3000));  // Min voltage
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(3300));  // Typical voltage
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(5000));  // Max voltage
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(4200));  // Mid-range voltage
}

void test_voltage_validation_invalid(void)
{
    TEST_ASSERT_FALSE(i2c_master_validate_voltage(2999));  // Below min
    TEST_ASSERT_FALSE(i2c_master_validate_voltage(5001));  // Above max
    TEST_ASSERT_FALSE(i2c_master_validate_voltage(2500));  // Too low
    TEST_ASSERT_FALSE(i2c_master_validate_voltage(6000));  // Too high
}

// Test voltage boundary conditions
void test_voltage_validation_boundaries(void)
{
    // Test exact boundaries
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(DISPLAY_VOLTAGE_MIN_MV));
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(DISPLAY_VOLTAGE_MAX_MV));
    TEST_ASSERT_TRUE(i2c_master_validate_voltage(DISPLAY_VOLTAGE_TYPICAL_MV));
    
    // Test just outside boundaries
    TEST_ASSERT_FALSE(i2c_master_validate_voltage(DISPLAY_VOLTAGE_MIN_MV - 1));
    TEST_ASSERT_FALSE(i2c_master_validate_voltage(DISPLAY_VOLTAGE_MAX_MV + 1));
}

// Test driver name retrieval
void test_driver_name_ssd1306(void)
{
    const char *name = i2c_master_get_driver_name(DISPLAY_DRIVER_SSD1306);
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_EQUAL_STRING("SSD1306", name);
}

void test_driver_name_ssd1315(void)
{
    const char *name = i2c_master_get_driver_name(DISPLAY_DRIVER_SSD1315);
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_EQUAL_STRING("SSD1315", name);
}

// Test I2C address constants
void test_i2c_address_constants(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x3C, OLED_I2C_ADDRESS_DEFAULT);
    TEST_ASSERT_EQUAL_HEX8(0x3D, OLED_I2C_ADDRESS_ALT);
}

// Test I2C clock speed constants
void test_i2c_clock_speed_constants(void)
{
    TEST_ASSERT_EQUAL(100000, I2C_MASTER_FREQ_HZ_STANDARD);
    TEST_ASSERT_EQUAL(400000, I2C_MASTER_FREQ_HZ_FAST);
}

// Test voltage constants
void test_voltage_constants(void)
{
    TEST_ASSERT_EQUAL(3000, DISPLAY_VOLTAGE_MIN_MV);
    TEST_ASSERT_EQUAL(5000, DISPLAY_VOLTAGE_MAX_MV);
    TEST_ASSERT_EQUAL(3300, DISPLAY_VOLTAGE_TYPICAL_MV);
    
    // Ensure min < typical < max
    TEST_ASSERT_LESS_THAN(DISPLAY_VOLTAGE_TYPICAL_MV, DISPLAY_VOLTAGE_MAX_MV);
    TEST_ASSERT_GREATER_THAN(DISPLAY_VOLTAGE_MIN_MV, DISPLAY_VOLTAGE_TYPICAL_MV);
}

// Test custom I2C configuration
void test_i2c_master_custom_config(void)
{
    i2c_master_config_t config = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_speed = 400000,
        .sda_pullup_en = false,
        .scl_pullup_en = false,
        .timeout_ms = 2000
    };
    
    TEST_ASSERT_EQUAL(I2C_NUM_1, config.i2c_port);
    TEST_ASSERT_EQUAL(GPIO_NUM_21, config.sda_io_num);
    TEST_ASSERT_EQUAL(GPIO_NUM_22, config.scl_io_num);
    TEST_ASSERT_EQUAL(400000, config.clk_speed);
    TEST_ASSERT_FALSE(config.sda_pullup_en);
    TEST_ASSERT_FALSE(config.scl_pullup_en);
    TEST_ASSERT_EQUAL(2000, config.timeout_ms);
}

// Test custom display configuration
void test_display_custom_config(void)
{
    display_config_t config = {
        .driver_ic = DISPLAY_DRIVER_SSD1315,
        .i2c_addr = 0x3D,
        .width = 128,
        .height = 32,
        .contrast = 0xFF,
        .low_power_mode = false
    };
    
    TEST_ASSERT_EQUAL(DISPLAY_DRIVER_SSD1315, config.driver_ic);
    TEST_ASSERT_EQUAL(0x3D, config.i2c_addr);
    TEST_ASSERT_EQUAL(128, config.width);
    TEST_ASSERT_EQUAL(32, config.height);
    TEST_ASSERT_EQUAL(0xFF, config.contrast);
    TEST_ASSERT_FALSE(config.low_power_mode);
}

// Register tests with Unity
void test_i2c_master_functions(void)
{
    RUN_TEST(test_i2c_master_config_default);
    RUN_TEST(test_display_config_default);
    RUN_TEST(test_voltage_validation_valid);
    RUN_TEST(test_voltage_validation_invalid);
    RUN_TEST(test_voltage_validation_boundaries);
    RUN_TEST(test_driver_name_ssd1306);
    RUN_TEST(test_driver_name_ssd1315);
    RUN_TEST(test_i2c_address_constants);
    RUN_TEST(test_i2c_clock_speed_constants);
    RUN_TEST(test_voltage_constants);
    RUN_TEST(test_i2c_master_custom_config);
    RUN_TEST(test_display_custom_config);
}
