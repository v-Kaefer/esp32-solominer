# ESP32 Mining Quick Start Guide

**TL;DR:** Fast setup guide for ESP32-S3 Bitcoin mining. Read [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md) for comprehensive details.

---

## ⚡ Quick Setup (5 Minutes)

### What You Need
- ESP32-S3 DevKitC (N16R8 recommended) - $8-10
- SSD1306 OLED Display (128x64, I2C) - $3-5  
- **40mm 5V Fan (STRONGLY RECOMMENDED)** - $3-5
- USB-C cable and computer with ESP-IDF

### Wiring
```
ESP32-S3 → OLED Display
GPIO 15  → SDA
GPIO 9   → SCL
3.3V     → VCC
GND      → GND

ESP32-S3 → Fan (optional but recommended)
5V       → Fan Red Wire
GND      → Fan Black Wire
```

### Build & Flash
```bash
cd esp32-solominer
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

---

## 🎯 Which Approach Should I Choose?

### Choose NerdMiner If:
- ✅ You're new to ESP32/Bitcoin mining
- ✅ You want easy setup with nice display
- ✅ You value community support
- ⚡ Expected: 20-30 kH/s

### Choose NMMiner If:
- ✅ You want maximum hashrate
- ✅ You can handle more complex setup
- ✅ You have active cooling (fan)
- ⚡ Expected: 40-60 kH/s

### Choose LeafMiner If:
- ✅ You want battery/solar operation
- ✅ Power efficiency is priority
- ✅ You don't need continuous mining
- ⚡ Expected: 5-15 kH/s (intermittent)

---

## ❄️ Cooling: Do I Need It?

| Your Setup | Cooling Needed | Why |
|------------|---------------|-----|
| Clock < 160MHz | Heatsink only | Low heat |
| Clock 200-240MHz, single core | **Fan recommended** | Prevents throttling |
| Clock 240MHz, dual core | **Fan mandatory** | High heat output |
| Mining array (4+ units) | **Fans mandatory** | Accumulated heat |
| Low-power mode (LeafMiner) | Heatsink only | Minimal heat |

**Rule of thumb:** If hashrate drops over time or ESP32 feels very hot, add a fan!

---

## 📊 Reality Check

### Will I Make Money?
**No.** ESP32 mining is NOT profitable. Here's why:

```
ESP32-S3:        40,000 H/s (40 kH/s)
Antminer S19:    110,000,000,000,000 H/s (110 TH/s)

Your ESP32 has: 0.00000004% of one ASIC's power
Expected years to find block: ~300,000 years
Daily earnings: ~$0.000001
```

### Then Why Mine?
- 🎓 **Learn** Bitcoin protocol deeply
- 🎲 **Lottery** mining - tiny chance of finding block!
- 🛠️ **Build** skills in embedded systems
- 🤝 **Support** Bitcoin network (symbolically)
- 🎉 **Fun** hobby project

---

## 🔥 Temperature Management

### Safe Temperatures
- ✅ **< 70°C:** Safe
- ⚠️ **70-80°C:** Warning
- 🔴 **> 80°C:** Throttling/danger

### Check Temperature
```c
#include "driver/temp_sensor.h"

float get_temp(void) {
    temp_sensor_config_t cfg = TEMP_SENSOR_CONFIG_DEFAULT();
    temp_sensor_set_config(cfg);
    temp_sensor_start();
    float temp;
    temp_sensor_read_celsius(&temp);
    temp_sensor_stop();
    return temp;
}
```

### If Overheating:
1. Add/improve fan
2. Reduce clock: 240MHz → 160MHz
3. Switch from dual-core to single-core
4. Add heatsink
5. Improve ambient airflow

---

## 🌐 Mining Pool Setup

### Recommended Pools

**Solo Mining (Lottery):**
```c
#define POOL_URL "solo.ckpool.org"
#define POOL_PORT 3333
```
- Keep full reward if block found
- Extremely rare to find blocks

**Pool Mining (Regular Payouts):**
```c
#define POOL_URL "stratum+tcp://stratum.braiins.com"
#define POOL_PORT 3333
```
- Small regular payouts
- More consistent results

### WiFi Config
```c
// config.h
#define WIFI_SSID "YourWiFi"
#define WIFI_PASS "YourPassword"
#define WALLET_ADDRESS "your_btc_address_here"
```

---

## 📈 Expected Performance

### Single ESP32-S3

| Mode | Clock | Cores | Cooling | Hashrate | Power |
|------|-------|-------|---------|----------|-------|
| Eco | 160MHz | 1 | Passive | 12 kH/s | 0.8W |
| Standard | 240MHz | 1 | Fan | 25 kH/s | 1.2W |
| Performance | 240MHz | 2 | Fan | 45 kH/s | 1.8W |
| Maximum | 240MHz+ | 2 | Fan | 55 kH/s | 2.2W |

### Mining Array (10 devices)
- Combined: ~450 kH/s
- Power: ~18W (with fans)
- Cost: ~$150
- Still not profitable, but more fun! 😄

---

## 🔧 Quick Optimization Tips

### 1. Enable Hardware SHA (Future)
```c
// ESP32-S3 has hardware SHA accelerator
// Can provide 2-5x speedup when properly implemented
// Check ESP-IDF documentation for latest support
```

### 2. Compiler Flags
```cmake
# CMakeLists.txt
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -funroll-loops")
```

### 3. Dual-Core Mining
```c
// Core 0: Nonce 0x00000000 - 0x7FFFFFFF
xTaskCreatePinnedToCore(mine_core0, "mine0", 8192, NULL, 5, NULL, 0);

// Core 1: Nonce 0x80000000 - 0xFFFFFFFF  
xTaskCreatePinnedToCore(mine_core1, "mine1", 8192, NULL, 5, NULL, 1);
```

### 4. Reduce Display Updates
```c
// Update every 5 seconds instead of 1 second
// Saves CPU cycles for mining
#define DISPLAY_UPDATE_INTERVAL_MS 5000
```

### 5. Use IRAM for Hot Code
```c
// Place frequently called functions in IRAM
IRAM_ATTR void sha256_inner_loop(void) {
    // Critical mining code here
}
```

---

## 🐛 Troubleshooting

### Problem: Low Hashrate
- ✅ Check clock frequency: Should be 240MHz
- ✅ Verify both cores are running (dual-core mode)
- ✅ Build with `-O3` optimization
- ✅ Check for thermal throttling

### Problem: Crashes/Reboots
- ✅ Add/improve cooling (likely overheating)
- ✅ Check power supply (need 2A+ for full load)
- ✅ Reduce clock speed
- ✅ Increase task stack size

### Problem: OLED Not Working
- ✅ Verify I2C pins (SDA=15, SCL=9)
- ✅ Check I2C address (try 0x3C and 0x3D)
- ✅ Test with I2C scanner
- ✅ Check pull-up resistors

### Problem: Can't Connect to Pool
- ✅ Verify WiFi credentials
- ✅ Test pool URL with ping
- ✅ Check firewall settings
- ✅ Try different pool

---

## 📚 Next Steps

1. **Start Mining:** Follow quick setup above
2. **Monitor Stats:** Watch hashrate and temperature
3. **Add Cooling:** Install fan if temps > 70°C
4. **Optimize Code:** Try dual-core mode
5. **Read Full Guide:** [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md)
6. **Join Community:** Discord, Reddit, GitHub

---

## 🎉 Pro Tips

### Tip #1: Start Simple
Don't optimize too early. Get basic mining working first, then improve.

### Tip #2: Monitor Temperature
Temperature is your #1 enemy. Watch it closely and add cooling proactively.

### Tip #3: Pool vs Solo
Start with a pool for regular feedback. Try solo mining after you understand the basics.

### Tip #4: Manage Expectations
You won't get rich. You will learn a lot. Enjoy the process!

### Tip #5: Try Duino-Coin
If you want actual (tiny) earnings, try Duino-Coin instead - it's designed for microcontrollers.

---

## 📞 Get Help

- **Issues:** GitHub Issues on this repository
- **Discussions:** GitHub Discussions
- **Reddit:** r/esp32, r/BitcoinMining
- **Discord:** NerdMiner community server
- **Full Documentation:** [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md)

---

**Happy Mining!** ⛏️🪙

Remember: The real treasure is the embedded systems skills you gain along the way! 🚀
