# Changelog

All notable changes to the ESP32 SoloMiner project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **ESP32-S3 Hardware SHA Acceleration**: Enabled hardware SHA accelerator for 2-5x mining performance improvement
- Comprehensive hardware SHA acceleration documentation (`HARDWARE_SHA_ACCELERATION.md`)
- Hardware SHA status logging at startup
- Performance optimization via `CONFIG_COMPILER_OPTIMIZATION_PERF`
- Modular I2C driver with SSD1306/SSD1315 display support
- Comprehensive unit test framework using ESP-IDF Unity
- Automated test coverage detection scripts
- GPIO pin test tool for I2C pin identification
- Documentation for mining strategies and quick start guide

### Changed
- I2C driver architecture: now modular and reusable
- Display initialization: supports both SSD1306 and SSD1315 driver ICs
- Pin configuration: Fixed I2C pins (SDA=GPIO15, SCL=GPIO9)
- WiFi configuration: now uses `config.h` pattern for security
- SHA-256 hashing: now uses hardware acceleration by default (transparent via mbedTLS)

### Fixed
- I2C driver initialization issues
- Display not responding due to incorrect pin mapping
- WiFi credential security (no longer hardcoded)

## [0.1.0] - Initial Release

### Added
- Basic ESP32-S3 Bitcoin mining implementation
- SSD1306 OLED display support
- WiFi connectivity
- Stratum protocol support (basic)
- Build system using ESP-IDF
- CI/CD workflows for build, test, and security scanning

### Project Goals
- Learning-focused Bitcoin mining project
- Pure software implementation (no external ASIC)
- Educational value over profitability
- Hardware optimization experiments
