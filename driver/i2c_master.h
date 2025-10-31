/**
 * @file i2c_master.h
 * @brief I2C Master Driver for SSD1306/SSD1315 OLED Displays
 * 
 * This driver provides modular I2C master configuration for 128x64 OLED display modules
 * with IIC interface. Supports both SSD1306 and SSD1315 driver ICs.
 * 
 * Hardware Specifications:
 * - Interface: 4-pin IIC (GND, VCC, SCL, SDA)
 * - Voltage Range: 3V ~ 5V DC
 * - Resolution: 128x64 pixels
 * - Power Consumption: Ultra-low power operation
 * - Display: High brightness and contrast
 * 
 * Supported Driver ICs:
 * - SSD1306 (Original)
 * - SSD1315 (Compatible variant with improved features)
 */

#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Display driver IC types
 */
typedef enum {
    DISPLAY_DRIVER_SSD1306,  /**< SSD1306 driver IC */
    DISPLAY_DRIVER_SSD1315   /**< SSD1315 driver IC (compatible with SSD1306) */
} display_driver_ic_t;

/**
 * @brief I2C master configuration structure
 */
typedef struct {
    i2c_port_t i2c_port;            /**< I2C port number */
    gpio_num_t sda_io_num;          /**< GPIO number for SDA */
    gpio_num_t scl_io_num;          /**< GPIO number for SCL */
    uint32_t clk_speed;             /**< I2C clock frequency in Hz */
    bool sda_pullup_en;             /**< Enable internal pullup for SDA */
    bool scl_pullup_en;             /**< Enable internal pullup for SCL */
    uint32_t timeout_ms;            /**< I2C operation timeout in milliseconds */
} i2c_master_config_t;

/**
 * @brief Display configuration structure
 */
typedef struct {
    display_driver_ic_t driver_ic;  /**< Display driver IC type */
    uint8_t i2c_addr;               /**< I2C device address (typically 0x3C or 0x3D) */
    int width;                      /**< Display width in pixels */
    int height;                     /**< Display height in pixels */
    uint8_t contrast;               /**< Display contrast (0-255) */
    bool low_power_mode;            /**< Enable ultra-low power operation */
} display_config_t;

/**
 * @brief Default I2C configuration for OLED displays
 */
#define I2C_MASTER_DEFAULT_CONFIG() {           \
    .i2c_port = I2C_NUM_0,                      \
    .sda_io_num = GPIO_NUM_15,                  \
    .scl_io_num = GPIO_NUM_9,                   \
    .clk_speed = 100000,                        \
    .sda_pullup_en = true,                      \
    .scl_pullup_en = true,                      \
    .timeout_ms = 1000                          \
}

/**
 * @brief Default display configuration for SSD1306/SSD1315
 */
#define DISPLAY_DEFAULT_CONFIG() {              \
    .driver_ic = DISPLAY_DRIVER_SSD1306,        \
    .i2c_addr = 0x3C,                           \
    .width = 128,                               \
    .height = 64,                               \
    .contrast = 0xCF,                           \
    .low_power_mode = true                      \
}

/**
 * @brief Voltage range constants (in millivolts)
 */
#define DISPLAY_VOLTAGE_MIN_MV    3000    /**< Minimum operating voltage: 3.0V */
#define DISPLAY_VOLTAGE_MAX_MV    5000    /**< Maximum operating voltage: 5.0V */
#define DISPLAY_VOLTAGE_TYPICAL_MV 3300   /**< Typical operating voltage: 3.3V */

/**
 * @brief Common I2C addresses for OLED displays
 */
#define OLED_I2C_ADDRESS_DEFAULT  0x3C    /**< Default I2C address */
#define OLED_I2C_ADDRESS_ALT      0x3D    /**< Alternate I2C address */

/**
 * @brief I2C clock speed constants
 */
#define I2C_MASTER_FREQ_HZ_STANDARD  100000   /**< Standard mode: 100 kHz */
#define I2C_MASTER_FREQ_HZ_FAST      400000   /**< Fast mode: 400 kHz */

/**
 * @brief Initialize I2C master with the given configuration
 * 
 * This function initializes the I2C master peripheral with the specified
 * configuration. It configures the GPIO pins, clock speed, and internal
 * pull-up resistors.
 * 
 * @param config Pointer to I2C master configuration structure
 * @return ESP_OK on success, error code otherwise
 * 
 * @note The I2C pins should be properly connected to the display module:
 *       - SDA (Data line)
 *       - SCL (Clock line)
 *       - External pull-up resistors (4.7kΩ typical) are recommended for better reliability
 */
esp_err_t i2c_master_init(const i2c_master_config_t *config);

/**
 * @brief Deinitialize I2C master
 * 
 * This function releases the I2C master resources and driver.
 * 
 * @param i2c_port I2C port number to deinitialize
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_deinit(i2c_port_t i2c_port);

/**
 * @brief Validate operating voltage for the display
 * 
 * Checks if the provided voltage is within the safe operating range (3V-5V DC).
 * This is important for ensuring the display operates correctly and safely.
 * 
 * @param voltage_mv Voltage in millivolts
 * @return true if voltage is within valid range, false otherwise
 */
bool i2c_master_validate_voltage(uint32_t voltage_mv);

/**
 * @brief Detect display driver IC type
 * 
 * Attempts to detect whether the connected display uses SSD1306 or SSD1315
 * driver IC by reading manufacturer/device information.
 * 
 * @param i2c_port I2C port number
 * @param i2c_addr I2C device address
 * @param driver_ic Pointer to store detected driver IC type
 * @return ESP_OK on success, error code otherwise
 * 
 * @note SSD1315 is largely compatible with SSD1306, so detection may default
 *       to SSD1306 if specific identification is not possible.
 */
esp_err_t i2c_master_detect_driver(i2c_port_t i2c_port, uint8_t i2c_addr, 
                                    display_driver_ic_t *driver_ic);

/**
 * @brief Check if a device is present at the specified I2C address
 * 
 * Performs a simple I2C write to check for device acknowledgment.
 * 
 * @param i2c_port I2C port number
 * @param i2c_addr I2C device address to check
 * @return ESP_OK if device responds, error code otherwise
 */
esp_err_t i2c_master_probe_device(i2c_port_t i2c_port, uint8_t i2c_addr);

/**
 * @brief Get display driver IC name as string
 * 
 * @param driver_ic Driver IC type
 * @return String representation of the driver IC name
 */
const char* i2c_master_get_driver_name(display_driver_ic_t driver_ic);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_MASTER_H__ */
