# ESP32 SoloMiner

Project to test a setup for ESP32 S3 N16R8 as a bitcoin solo miner or ticket miner.

IDE: VSCode, using the ESP-IDF: Explorer extension.

Project first auto build for ESP32, then changed to ESP32S3 (via ESP-PROG).

## Features

- Bitcoin SHA256 double hashing implementation
- Real-time mining statistics display on SSD1306 OLED
- WiFi connectivity for network communication
- I2C pin auto-detection for easy hardware setup
- FreeRTOS-based mining task on dedicated core

## Hardware Requirements

- ESP32-S3 N16R8 development board
- SSD1306 128x64 OLED display (I2C)
- WiFi connection

## Documentation

This project includes auto-generated documentation using Doxygen.

### Generating Documentation

To generate the HTML documentation:

```bash
doxygen Doxyfile
```

The documentation will be generated in the `docs/html/` directory.

### Viewing Documentation

Open `docs/html/index.html` in your web browser to view the complete API documentation, including:

- Detailed function descriptions
- Parameter documentation
- Return value specifications
- Call graphs and dependency diagrams
- File and structure documentation

### Installing Doxygen

If you don't have Doxygen installed:

**Ubuntu/Debian:**
```bash
sudo apt-get install doxygen graphviz
```

**macOS:**
```bash
brew install doxygen graphviz
```

**Windows:**
Download from [doxygen.nl](https://www.doxygen.nl/download.html)

## Building the Project

1. Set up ESP-IDF environment
2. Create a `.env` file with WiFi credentials
3. Build with `idf.py build`
4. Flash with `idf.py flash`

## Configuration

Edit the `.env` file to configure:
- `WIFI_SSID_VAR`: Your WiFi SSID
- `WIFI_PASS_VAR`: Your WiFi password

Adjust I2C pins in `main/main.c` if needed:
- `I2C_MASTER_SCL_IO`: SCL GPIO pin (default: 8)
- `I2C_MASTER_SDA_IO`: SDA GPIO pin (default: 15)

## License

This is an educational/testing project for learning about Bitcoin mining and ESP32 development.

**Note:** The ESP32's hashrate is far too low for practical Bitcoin mining. This is for educational purposes only.