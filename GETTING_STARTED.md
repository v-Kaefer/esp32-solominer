# Getting Started with Real Bitcoin Mining

This guide will help you configure and start actual Bitcoin mining on your ESP32-S3 device.

## What You Need

1. **Hardware:**
   - ESP32-S3 DevKit (N16R8 recommended)
   - SSD1306 OLED Display (128x64, I2C)
   - USB-C cable
   - **Optional but recommended:** Small 5V cooling fan

2. **Software:**
   - ESP-IDF v5.1.2 or later installed
   - This project cloned and built

3. **Configuration:**
   - WiFi credentials
   - Bitcoin wallet address
   - Mining pool selection

## Step-by-Step Setup

### 1. Configure WiFi and Mining Pool

Copy the example configuration file:
```bash
cp main/config.h.example main/config.h
```

Edit `main/config.h` with your settings:
```c
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASS "YourWiFiPassword"

// Choose your mining pool
#define POOL_URL "solo.ckpool.org"      // For solo mining (lottery style)
// #define POOL_URL "stratum.braiins.com"  // For pool mining (regular payouts)

#define POOL_PORT 3333

// Your Bitcoin address where rewards will be sent
#define WALLET_ADDRESS "your_bitcoin_address_here"

#define WORKER_NAME "ESP32Miner"
```

### 2. Choose Your Mining Strategy

#### Solo Mining (Lottery Style)
```c
#define POOL_URL "solo.ckpool.org"
#define POOL_PORT 3333
```
- **Pros:** Keep full block reward (~6.25 BTC + fees) if you find a block
- **Cons:** Extremely unlikely to find a block (~300,000 years with one ESP32)
- **Best for:** Learning, lottery-style participation

#### Pool Mining (Regular Payouts)
```c
#define POOL_URL "stratum.braiins.com"
#define POOL_PORT 3333
```
- **Pros:** Small regular payouts based on contributed hashrate
- **Cons:** Earnings will be extremely tiny ($0.000001/day per device)
- **Best for:** Seeing actual results, testing

### 3. Get a Bitcoin Wallet Address

If you don't have a Bitcoin address:

1. **For testing:** Create a testnet address
   - Use a testnet faucet to get test coins
   - Configure pool: `solo.ckpool.org:13333` (testnet)

2. **For real mining:** Use a reputable wallet
   - Hardware wallet (Ledger, Trezor) - safest
   - Software wallet (Electrum, BlueWallet)
   - **Never** use exchange addresses for mining!

### 4. Build and Flash

```bash
# Set target to ESP32-S3
idf.py set-target esp32s3

# Build the project
idf.py build

# Flash to device and monitor
idf.py flash monitor
```

### 5. Monitor Your Miner

Once running, the OLED display will show:
```
ESP32-S3 BTC Miner
Pool: Connected
Rate: 25.3 H/s
Shares: 0
Best: 28 zeros
Nonce: 1234567
```

Serial monitor output will show:
```
I (12345) BTC_MINER: ESP32-S3 Bitcoin Miner Starting...
I (12346) BTC_MINER: Dual-Core Architecture: Core 0=Mining, Core 1=I/O
I (12500) BTC_MINER: WiFi init finished.
I (12600) STRATUM: Connecting to solo.ckpool.org:3333
I (12800) STRATUM: Connected to pool
I (12900) STRATUM: Subscribed. Extranonce1: 12345678, size: 4
I (13000) STRATUM: Authorized successfully
I (13100) BTC_MINER: New mining job received: job123
I (15000) BTC_MINER: Hashrate: 25.3 H/s, Total: 50600, Best: 28
```

## Understanding the Output

### Display Indicators

- **Pool: Connected** - Successfully connected and authorized with mining pool
- **Pool: Connecting..** - Attempting to connect to pool
- **Rate: XX.X H/s** - Current hashrate (hashes per second)
- **Shares: XX** - Number of valid shares submitted to pool
- **Best: XX zeros** - Best difficulty found (leading zero bits in hash)
- **Nonce: XXXXXX** - Current nonce being tested

### What to Expect

**Realistic Performance:**
- **Hashrate:** 20-50 H/s (kH/s) depending on configuration
- **Power consumption:** 1-2W
- **Temperature:** 60-80°C (add cooling if > 70°C)
- **Shares found:** Every few hours to days (depending on pool difficulty)
- **Blocks found:** Extremely unlikely (~300,000 years solo mining)

**Share Submissions:**
Pool mining pools typically set difficulty around 2^32 (32 leading zeros). With 25 H/s:
- Expected time to find a share: ~171,798,691 seconds = ~5.4 years
- With difficulty adjustment, shares may come more frequently

## Troubleshooting

### "Pool: Connecting.." Never Changes to "Connected"

**Check:**
1. WiFi credentials are correct
2. Internet connection is working
3. Pool URL and port are correct
4. Firewall allows outgoing connection on port 3333

**Test manually:**
```bash
telnet solo.ckpool.org 3333
```

### Low Hashrate (< 10 H/s)

**Causes:**
1. Build not optimized (`-O3` flag missing)
2. Display updates too frequent (reduce to 5 seconds)
3. Thermal throttling (add cooling)
4. Clock speed too low (should be 240 MHz)

**Fix:**
Check CMakeLists.txt has optimization flags:
```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -funroll-loops")
```

### Device Crashes or Reboots

**Causes:**
1. Overheating (most common)
2. Insufficient power supply
3. Stack overflow in tasks

**Fix:**
1. Add cooling fan
2. Use 2A+ power supply
3. Reduce clock speed if persistent

### No Shares Found After Days

**This is normal!** Expected behavior:
- Solo mining: Shares = blocks, extremely rare
- Pool mining: May take days/weeks depending on pool difficulty
- ESP32 hashrate is 1/1,000,000,000,000 of modern ASIC

## Advanced Configuration

### Optimizing for Maximum Hashrate

1. **Enable dual-core mining** (coming in future update)
2. **Reduce display updates:**
   ```c
   // In display_io_task
   vTaskDelay(pdMS_TO_TICKS(5000)); // 5 seconds instead of 2
   ```
3. **Overclock (risky):**
   ```c
   // In sdkconfig or menuconfig
   CONFIG_ESP32S3_DEFAULT_CPU_FREQ_240=y
   ```
4. **Add active cooling** - mandatory for sustained high performance

### Testing with Testnet

To test without risking real Bitcoin:

```c
#define POOL_URL "solo.ckpool.org"
#define POOL_PORT 13333  // Testnet port
#define WALLET_ADDRESS "your_testnet_address"
```

Get testnet coins from faucets to test pool payouts.

## Understanding Bitcoin Mining

### How It Works

1. **Pool Connection:**
   - ESP32 connects to mining pool via Stratum protocol
   - Pool sends work (block template) to miner

2. **Mining Process:**
   - Construct block header from pool data
   - Try different nonces (0 to 4,294,967,295)
   - Calculate double SHA-256 hash
   - Check if hash meets difficulty target

3. **Share Submission:**
   - If hash meets pool difficulty, submit as "share"
   - Pool tracks your contributions
   - Rewards distributed based on shares

4. **Block Finding:**
   - If hash meets Bitcoin network difficulty (~80 zeros)
   - Block is broadcast to Bitcoin network
   - Miner receives block reward (~6.25 BTC + fees)

### Why Mine with ESP32?

**Not for profit** - this cannot be emphasized enough!

**For learning:**
- Understand Bitcoin protocol deeply
- Learn embedded systems programming
- Experiment with optimization techniques
- Support Bitcoin network (symbolically)
- Fun hobby project with tiny lottery chance

### Mining Economics

**Cost vs. Revenue (per ESP32-S3):**
- Power cost: ~$0.02/day (at $0.12/kWh)
- Expected revenue: ~$0.000001/day
- **Net loss: $0.02/day**

**Breaking even would require:**
- 20,000x more hashrate (impossible with current hardware)
- OR Bitcoin price to increase 20,000x
- OR electricity to be free

**But:**
- Learning experience: Priceless
- Supporting Bitcoin: Symbolic value
- Finding a block: Lottery ticket (~1 in 100 million chance per device-lifetime)

## Next Steps

1. ✅ Get basic mining working
2. ✅ Monitor temperature and hashrate
3. 🔄 Optimize performance if desired
4. 🔄 Consider building a small array (4-10 devices)
5. 🔄 Experiment with different pools
6. 📚 Learn more about Bitcoin and cryptography

## Support

- **GitHub Issues:** Report bugs or ask questions
- **Documentation:** See ESP32_MINING_STRATEGIES.md for comprehensive guide
- **Community:** Join r/BitcoinMining, r/esp32

---

**Remember:** ESP32 Bitcoin mining is for **education and fun**, not profit!

The real treasure is the knowledge you gain along the way. 🚀⛏️
