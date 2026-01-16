# Changelog

All notable changes to the ESP32 SoloMiner project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Stratum V1 Protocol Client** - Full implementation for real Bitcoin mining pool connectivity
- **Real Mining Support** - Connect to pools like solo.ckpool.org, Braiins Pool, Slush Pool
- **Mining Job Management** - Receive and process mining.notify messages from pools
- **Share Submission** - Submit found shares back to mining pools
- **Block Header Construction** - Build real block headers from pool work data
- **Coinbase Transaction Handling** - Proper coinbase construction with extranonce support
- **Merkle Root Calculation** - Calculate merkle roots from pool-provided merkle branches
- **Pool Authentication** - mining.subscribe and mining.authorize implementation
- **Difficulty Tracking** - Monitor and adapt to pool difficulty changes
- **Connection Management** - Automatic reconnection with retry logic
- **Enhanced Display** - Show pool connection status and accepted shares
- **Comprehensive Getting Started Guide** - Complete setup documentation for real mining
- Modular I2C driver with SSD1306/SSD1315 display support
- Comprehensive unit test framework using ESP-IDF Unity
- Automated test coverage detection scripts
- GPIO pin test tool for I2C pin identification
- Documentation for mining strategies and quick start guide

### Changed
- **Mining Architecture** - Now uses real pool data instead of mock block headers
- **Configuration System** - Added pool URL, port, and wallet address configuration
- **Task Organization** - Added dedicated Stratum client task on Core 1
- **Statistics Tracking** - Added shares submitted/accepted counters
- **Display Updates** - Show pool connection status instead of generic separator
- I2C driver architecture: now modular and reusable
- Display initialization: supports both SSD1306 and SSD1315 driver ICs
- Pin configuration: Fixed I2C pins (SDA=GPIO15, SCL=GPIO9)
- WiFi configuration: now uses `config.h` pattern for security

### Fixed
- **Mining Functionality** - Miner now connects to real pools instead of mining in isolation
- **Work Management** - Proper handling of mining jobs and nonce ranges
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
