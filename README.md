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

To configure your WiFi credentials for local development:

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

**Note:** Never commit your `main/config.h` file with real credentials to version control.

## MQTT Support

The miner includes MQTT support for real-time monitoring of mining statistics. MQTT runs on the secondary processor (Core 0) to ensure it doesn't disrupt SHA256 mining operations running on Core 1.

### Features

- Real-time publishing of mining statistics:
  - Hashrate (H/s)
  - Total hashes computed
  - Best difficulty found
  - Miner status updates

### Configuration

MQTT settings are configured in `main/config.h`. By default, it uses the public EMQX broker at `broker.emqx.io`:

```c
#define MQTT_BROKER_URL "mqtt://broker.emqx.io:1883"
#define MQTT_CLIENT_ID "esp32_btc_miner"
#define ENABLE_MQTT 1  // Set to 0 to disable MQTT
```

### MQTT Topics

The miner publishes to the following topics:

- `btc_miner/hashrate` - Current hashrate in H/s
- `btc_miner/total_hashes` - Total hashes computed since start
- `btc_miner/best_difficulty` - Best difficulty found (leading zeros)
- `btc_miner/status` - Status messages (online/offline)

### Testing with MQTTX

For testing and monitoring MQTT messages, we recommend using **MQTTX AIO Toolbox**:

- **MQTTX Desktop**: [https://github.com/emqx/MQTTX](https://github.com/emqx/MQTTX)
- **MQTTX Web**: [http://mqtt-client.emqx.com/](http://mqtt-client.emqx.com/)

**Note**: MQTTX AIO Toolbox provides a user-friendly interface for subscribing to topics and monitoring real-time data from your miner.

To monitor your miner:
1. Open MQTTX and connect to your MQTT broker
2. Subscribe to `btc_miner/#` to see all topics
3. Watch real-time mining statistics update every 2 seconds

### CI/CD Builds

CI/CD builds automatically skip WiFi functionality since `config.h` is not committed to the repository for security reasons. The WiFi code is conditionally compiled only when `WIFI_SSID` is defined (which comes from your local `config.h` file created from `config.h.example`). This allows automated builds to succeed without requiring WiFi credentials.

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
