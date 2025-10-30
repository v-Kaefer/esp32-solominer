Project to test a setup for ESP32 S3 N16R8 as a bitcoin solo miner or ticket miner.

IDE: VSCode, using the ESP-IDF: Explorer extension.

Project first auto build for ESP32, then changed to ESP32S3 (via ESP-PROG).

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
