# ESP32-S3 Bitcoin Solo Miner

Project to test a setup for ESP32 S3 N16R8 as a bitcoin solo miner or ticket miner.

[![Build](https://github.com/v-Kaefer/esp32-solominer/actions/workflows/build.yml/badge.svg)](https://github.com/v-Kaefer/esp32-solominer/actions/workflows/build.yml)
[![CodeQL](https://github.com/v-Kaefer/esp32-solominer/actions/workflows/codeql.yml/badge.svg)](https://github.com/v-Kaefer/esp32-solominer/actions/workflows/codeql.yml)
[![Static Analysis](https://github.com/v-Kaefer/esp32-solominer/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/v-Kaefer/esp32-solominer/actions/workflows/static-analysis.yml)

**IDE:** VSCode, using the ESP-IDF: Explorer extension.

**Hardware:** ESP32-S3-N16R8 (via ESP-PROG)

## 🎯 Project Goals

This is a learning project focused on:
- Learning C and Assembly programming
- Developing low-level, hardware-effective code
- Ensuring safety, privacy, and modularity
- Building an ESP32 Bitcoin miner from scratch

## WiFi Configuration

To configure your WiFi credentials:

1. Copy the example configuration file:
   ```bash
   cp main/config.h.example main/config.h
   ```

2. Edit `main/config.h` and update the WiFi credentials:
   ```c
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASS "your_wifi_password"
   ```

3. The `config.h` file is gitignored to prevent accidentally committing your credentials.

Note: Never commit your `main/config.h` file with real credentials to version control.

## Build Instructions

1. Install ESP-IDF v5.1.2 or later
2. Set up your WiFi configuration (see above)
3. Build the project:
   ```bash
   idf.py set-target esp32s3
   idf.py build
   ```
4. Flash to your ESP32-S3:
   ```bash
   idf.py flash monitor
   ```

## CI/CD

This project uses GitHub Actions for continuous integration. See [CI_CD_SETUP.md](CI_CD_SETUP.md) for details.

All pull requests must pass:
- ✅ Build verification
- 🔍 Static code analysis
- 🔒 Security scanning
- 📝 Code quality checks

## Contributing

See our [pull request template](.github/pull_request_template.md) and [issue templates](.github/ISSUE_TEMPLATE/) for contribution guidelines.

## License

This is a learning project. Feel free to use and modify for educational purposes.
