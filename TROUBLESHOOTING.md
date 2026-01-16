# Troubleshooting Guide

## Common Issues and Solutions

### Build Issues

#### Error: "cJSON.h: No such file or directory"

**Cause:** ESP-IDF's cJSON component not found.

**Solution:**
The `json` component is required in CMakeLists.txt. Verify:
```cmake
idf_component_register(
    ...
    REQUIRES json
)
```

This is already configured in `main/CMakeLists.txt`. If error persists, update ESP-IDF:
```bash
cd $IDF_PATH
git pull
git submodule update --init --recursive
```

#### Error: "undefined reference to stratum_*"

**Cause:** Linker cannot find stratum functions.

**Solution:**
Ensure `stratum.c` is listed in CMakeLists.txt:
```cmake
SRCS "main.c" "ssd1306.c" "stratum.c" "mining.c" "../driver/i2c_master.c"
```

#### Warning: "implicit declaration of function"

**Cause:** Missing header include.

**Solution:**
Check that all required headers are included. Each file should include its dependencies.

### Runtime Issues

#### Display shows "Pool: Connecting.." forever

**Symptoms:**
- Display never changes to "Pool: Connected"
- No "Subscribed" or "Authorized" logs in serial output

**Debugging:**

1. **Check WiFi connection:**
```
I (12500) BTC_MINER: WiFi init finished.
I (13000) BTC_MINER: Got IP:192.168.1.xxx
```
If no IP received, check WiFi credentials in `config.h`.

2. **Check DNS resolution:**
```
I (15000) STRATUM: Connecting to solo.ckpool.org:3333
```
If "DNS lookup failed" appears, check internet connection.

3. **Check pool connection:**
```
I (15200) STRATUM: Connected to pool
I (15300) STRATUM: Sent: {"id":1,"method":"mining.subscribe"...
```
If "Connection failed" appears, pool may be down or port blocked.

4. **Test pool manually:**
```bash
# On your computer
telnet solo.ckpool.org 3333
```
Should connect. Type anything and press Enter to test.

#### Miner crashes with "Task watchdog got triggered"

**Symptoms:**
```
E (45678) task_wdt: Task watchdog got triggered. The following tasks did not reset the watchdog in time:
```

**Causes:**
1. Mining loop running too long without yielding
2. Stack overflow in a task
3. Deadlock or infinite loop

**Solutions:**

1. **Increase watchdog timeout** (temporary):
```bash
idf.py menuconfig
# Component config → ESP System Settings → Task Watchdog timeout period
# Increase from 5 to 10 seconds
```

2. **Add more yield points:**
```c
// In mining_task, reduce yield frequency
if (local_nonce % 500 == 0) {  // Changed from 1000
    vTaskDelay(1);
}
```

3. **Increase task stack size:**
```c
xTaskCreatePinnedToCore(
    mining_task,
    "mining_task",
    16384,  // Increased from 8192
    ...
);
```

#### Crash with "Guru Meditation Error"

**Symptoms:**
```
Guru Meditation Error: Core 0 panic'ed (LoadProhibited)
```

**Causes:**
1. Null pointer dereference
2. Invalid memory access
3. Stack overflow

**Debugging:**

1. **Enable core dumps:**
```bash
idf.py menuconfig
# Component config → Core dump → Core dump destination → Flash
```

2. **Check backtrace:**
The error will show a backtrace like:
```
Backtrace: 0x400xxxxx:0x3ffxxxxx 0x400xxxxx:0x3ffxxxxx
```

Decode with:
```bash
xtensa-esp32s3-elf-addr2line -e build/esp32-solominer.elf 0x400xxxxx
```

3. **Common fixes:**
   - Check all pointers before dereferencing
   - Ensure buffers are large enough
   - Verify mutex is created before use

#### Low hashrate (< 10 H/s)

**Symptoms:**
```
I (60000) BTC_MINER: Hashrate: 5.2 H/s, Total: 10400, Best: 24
```

**Causes:**
1. Not built with optimizations
2. Thermal throttling
3. Display updates too frequent
4. WiFi/network overhead

**Solutions:**

1. **Verify optimization flags:**
```bash
# Check build output for -O3 flag
idf.py build | grep "\-O"
```

2. **Reduce display updates:**
```c
// In display_io_task
vTaskDelay(pdMS_TO_TICKS(5000));  // 5 seconds instead of 2
```

3. **Check temperature:**
```c
// Add temp monitoring (if available)
float temp = /* read from sensor */;
ESP_LOGI(TAG, "Temperature: %.1f°C", temp);
```

4. **Monitor task CPU usage:**
```bash
# In menuconfig, enable:
# Component config → FreeRTOS → Enable FreeRTOS to collect run time stats
```

#### Shares never accepted

**Symptoms:**
```
I (120000) BTC_MINER: Hashrate: 25.3 H/s, Total: 3036000, Best: 28
# Shares: 0 never changes
```

**Analysis:**

With 25 H/s hashrate and typical pool difficulty (2^32), expected time to find a share:
```
Time = 2^32 / 25 = 171,798,691 seconds = 5.4 years
```

**This is NORMAL!** ESP32 is extremely slow compared to ASICs.

**To test share submission:**

1. **Use testnet with lower difficulty:**
```c
#define POOL_URL "solo.ckpool.org"
#define POOL_PORT 13333  // Testnet
```

2. **Run longer:** Even on testnet, may take days to find a share.

3. **Check logs for submission attempts:**
```
I (xxx) BTC_MINER: !!! SHARE FOUND !!! Difficulty: 32
I (xxx) BTC_MINER: Share submitted successfully
```

#### Memory allocation failures

**Symptoms:**
```
E (45000) BTC_MINER: Failed to create statistics mutex!
E (45001) heap: alloc failed
```

**Causes:**
1. Heap exhausted
2. Memory fragmentation
3. Memory leak

**Solutions:**

1. **Check heap usage:**
```c
ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
```

2. **Reduce allocations:**
   - Use static buffers instead of malloc
   - Reduce task stack sizes if too large
   - Free unused resources

3. **Check for leaks:**
   - Ensure cJSON_Delete() is called for all cJSON_Parse()
   - Verify all malloc() have corresponding free()
   - Use static analysis tools

### Network Issues

#### "Connection refused" or "Connection reset"

**Causes:**
1. Pool is down
2. Wrong port
3. Firewall blocking
4. Too many connection attempts (rate limiting)

**Solutions:**

1. **Try different pool:**
```c
// Instead of solo.ckpool.org
#define POOL_URL "stratum.braiins.com"
```

2. **Check pool status:**
Visit pool website or use:
```bash
ping solo.ckpool.org
telnet solo.ckpool.org 3333
```

3. **Add connection delay:**
```c
// In stratum_task, increase retry delay
vTaskDelay(pdMS_TO_TICKS(60000));  // 60 seconds
```

#### "DNS lookup failed"

**Causes:**
1. No internet connection
2. DNS server not configured
3. WiFi not connected

**Solutions:**

1. **Check WiFi status:**
```c
wifi_ap_record_t ap_info;
esp_wifi_sta_get_ap_info(&ap_info);
ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);
```

2. **Use IP address instead:**
```c
// Get IP: ping solo.ckpool.org
#define POOL_URL "178.128.172.232"  // Example - check current IP
```

3. **Check DNS configuration:**
```bash
idf.py menuconfig
# Component config → LWIP → DNS → DNS server address
```

### Hardware Issues

#### Device overheating (> 80°C)

**Solutions:**
1. **Add active cooling** (fan)
2. **Reduce clock speed:**
```bash
idf.py menuconfig
# Component config → ESP32S3-Specific → CPU frequency
# Set to 160MHz instead of 240MHz
```
3. **Use single core instead of dual core**
4. **Improve ventilation**

#### Random reboots

**Causes:**
1. Overheating
2. Power supply insufficient
3. Brownout

**Solutions:**
1. Use 2A+ power supply
2. Add cooling
3. Check voltage under load
4. Reduce power consumption (lower clock)

## Debugging Tools

### Serial Monitor

Enable verbose logging:
```bash
idf.py menuconfig
# Component config → Log output → Default log verbosity → Verbose
```

### Memory Debugging

```c
// Add to main.c
#include "esp_heap_trace.h"

void monitor_heap(void) {
    ESP_LOGI(TAG, "Free heap: %lu, Min free: %lu",
             esp_get_free_heap_size(),
             esp_get_minimum_free_heap_size());
}
```

### Network Packet Capture

Use Wireshark on your computer to capture traffic between ESP32 and pool:
```bash
# On Linux/Mac
sudo tcpdump -i wlan0 port 3333 -w mining.pcap
```

Then analyze with Wireshark to see Stratum messages.

## Getting Help

If issue persists after trying these solutions:

1. **Enable verbose logging** and capture full output
2. **Note exact error messages** and conditions
3. **Document steps to reproduce**
4. **Check hardware specifications** (board variant, connections)
5. **Try minimal test case** if possible

**Report issue with:**
- ESP-IDF version
- Board type and variant
- Full serial output (with verbose logging)
- Configuration (sanitized - remove credentials)
- Steps to reproduce

Open an issue on GitHub with this information.

## Performance Optimization Checklist

- [ ] Build with `-O3` optimization
- [ ] Reduce display update frequency
- [ ] Enable compiler optimizations
- [ ] Use IRAM for hot functions (future enhancement)
- [ ] Add active cooling for sustained operation
- [ ] Monitor and manage temperature
- [ ] Reduce logging frequency in mining loop
- [ ] Consider dual-core mining (future enhancement)

---

**Remember:** ESP32 mining is educational. Many "issues" are actually expected behavior due to the huge disparity between ESP32 and ASIC miners!
