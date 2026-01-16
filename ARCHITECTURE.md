# System Architecture: Stratum Mining Implementation

## Overview

This document describes the architecture of the ESP32-S3 Bitcoin mining implementation with Stratum V1 protocol support.

## System Components

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32-S3                                 │
│                                                                   │
│  ┌────────────────────────────┐  ┌────────────────────────────┐│
│  │         Core 0             │  │         Core 1             ││
│  │      (Mining)              │  │      (I/O & Network)       ││
│  │                            │  │                            ││
│  │  ┌──────────────────────┐ │  │  ┌──────────────────────┐ ││
│  │  │   Mining Task        │ │  │  │   Stratum Task       │ ││
│  │  │  - Get jobs          │ │  │  │  - Connect to pool   │ ││
│  │  │  - Build headers     │ │  │  │  - Subscribe         │ ││
│  │  │  - SHA-256d hash     │ │  │  │  - Authorize         │ ││
│  │  │  - Check difficulty  │ │  │  │  - Receive jobs      │ ││
│  │  │  - Submit shares     │ │  │  │  - Submit shares     │ ││
│  │  │  Priority: 5         │ │  │  │  Priority: 4         │ ││
│  │  └──────────────────────┘ │  │  └──────────────────────┘ ││
│  │                            │  │                            ││
│  │                            │  │  ┌──────────────────────┐ ││
│  │                            │  │  │  Display/IO Task     │ ││
│  │                            │  │  │  - Update OLED       │ ││
│  │                            │  │  │  - Show stats        │ ││
│  │                            │  │  │  Priority: 3         │ ││
│  │                            │  │  └──────────────────────┘ ││
│  │                            │  │                            ││
│  │                            │  │  ┌──────────────────────┐ ││
│  │                            │  │  │   WiFi Stack         │ ││
│  │                            │  │  │  - TCP/IP (LwIP)     │ ││
│  │                            │  │  │  - Sockets           │ ││
│  │                            │  │  └──────────────────────┘ ││
│  └────────────────────────────┘  └────────────────────────────┘│
│                                                                   │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │             Shared Resources (Mutex Protected)              │ │
│  │  - Mining statistics  - Current job  - Pool status          │ │
│  └────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌─────────────────────┐
                    │   Mining Pool       │
                    │  (solo.ckpool.org)  │
                    │   Stratum V1        │
                    └─────────────────────┘
```

## Data Flow

### 1. Startup Sequence

```
app_main()
    │
    ├─► Initialize NVS
    ├─► Initialize I2C & Display
    ├─► Initialize WiFi (if WIFI_SSID defined)
    ├─► Initialize Stratum client
    ├─► Create tasks:
    │   ├─► Stratum Task (Core 1, Priority 4)
    │   ├─► Mining Task (Core 0, Priority 5)
    │   └─► Display Task (Core 1, Priority 3)
    └─► Return (tasks run independently)
```

### 2. Stratum Connection Flow

```
Stratum Task (Core 1)
    │
    ├─► Connect to pool (TCP socket)
    │   └─► DNS lookup → TCP connect
    │
    ├─► Send: mining.subscribe
    │   {"id":1,"method":"mining.subscribe","params":["ESP32Miner/1.0",null]}
    │   ◄── Response: {"id":1,"result":[[...],extranonce1,extranonce2_size]}
    │
    ├─► Send: mining.authorize
    │   {"id":2,"method":"mining.authorize","params":["wallet_address","x"]}
    │   ◄── Response: {"id":2,"result":true}
    │
    └─► Listen loop:
        ├─► Receive: mining.notify (new job)
        │   {"method":"mining.notify","params":[job_id,prevhash,coinb1,coinb2,
        │                                         merkle_branch,version,nbits,ntime,clean]}
        │   └─► Store in current_job (mutex protected)
        │
        ├─► Receive: mining.set_difficulty
        │   {"method":"mining.set_difficulty","params":[difficulty]}
        │   └─► Update current_difficulty
        │
        └─► Send: mining.submit (when share found)
            {"id":N,"method":"mining.submit","params":[worker,job_id,
                                                         extranonce2,ntime,nonce]}
```

### 3. Mining Loop Flow

```
Mining Task (Core 0)
    │
    └─► Loop forever:
        │
        ├─► Get current job from Stratum (mutex)
        │   └─► If no job or not authorized: wait 1s, continue
        │
        ├─► Build block header:
        │   │
        │   ├─► Construct coinbase transaction:
        │   │   coinbase = coinb1 + extranonce1 + extranonce2 + coinb2
        │   │   coinbase_hash = SHA256d(coinbase)
        │   │
        │   ├─► Calculate merkle root:
        │   │   merkle_root = coinbase_hash
        │   │   for each branch in merkle_branches:
        │   │       merkle_root = SHA256d(merkle_root + branch)
        │   │
        │   └─► Assemble 80-byte header:
        │       [0:3]   version
        │       [4:35]  prevhash
        │       [36:67] merkle_root
        │       [68:71] ntime
        │       [72:75] nbits
        │       [76:79] nonce
        │
        ├─► Hash block header:
        │   hash = SHA256d(header)
        │
        ├─► Count leading zeros in hash
        │
        ├─► Check if meets pool difficulty:
        │   if leading_zeros >= required:
        │       submit_share(job_id, extranonce2, ntime, nonce)
        │       increment shares counter
        │
        ├─► Increment nonce (0 to 2^32-1)
        │   if nonce overflow: increment extranonce2
        │
        ├─► Update statistics (every 2 seconds):
        │   └─► Calculate hashrate, update shared stats
        │
        └─► Yield to watchdog (every 1000 hashes)
```

### 4. Display Update Flow

```
Display Task (Core 1)
    │
    └─► Loop forever:
        │
        ├─► Read shared statistics (mutex)
        │   - hashrate
        │   - shares accepted
        │   - best difficulty
        │   - pool connection status
        │
        ├─► Update OLED display:
        │   Line 0: "ESP32-S3 BTC Miner"
        │   Line 1: "Pool: Connected" or "Pool: Connecting.."
        │   Line 2: "Rate: 25.3 H/s"
        │   Line 3: "Shares: 0"
        │   Line 4: "Best: 28 zeros"
        │   Line 5: "Nonce: 1234567"
        │
        └─► Wait 2 seconds
```

## Module Descriptions

### stratum.c/h - Stratum Protocol Client

**Responsibilities:**
- TCP socket connection to mining pool
- JSON-RPC message encoding/decoding (using cJSON)
- Stratum protocol state machine
- Job storage and synchronization

**Key Functions:**
- `stratum_init()` - Initialize client with pool config
- `stratum_connect()` - Establish TCP connection
- `stratum_task()` - Main event loop (runs on Core 1)
- `stratum_get_job()` - Thread-safe job retrieval
- `stratum_submit_share()` - Submit found share
- `stratum_handle_message()` - Parse and dispatch incoming messages

**State Machine:**
```
DISCONNECTED → CONNECTING → CONNECTED → SUBSCRIBED → AUTHORIZED
     ↑              ↓                                      │
     └──────────── ERROR ←──────────────────────────────┘
```

### mining.c/h - Mining Utilities

**Responsibilities:**
- Block header construction from Stratum job
- Coinbase transaction building
- Merkle root calculation
- Hex/byte conversion utilities

**Key Functions:**
- `mining_build_block_header()` - Construct 80-byte header
- `mining_calculate_merkle_root()` - Apply merkle branches
- `sha256d()` - Double SHA-256 hashing
- `hex_to_bytes()` / `bytes_to_hex()` - Conversion utilities

### main.c - Core Application

**Responsibilities:**
- System initialization
- Task creation and management
- SHA-256 mining loop
- Display updates
- Statistics tracking

**Key Functions:**
- `app_main()` - Application entry point
- `mining_task()` - Core 0 mining loop
- `display_io_task()` - Core 1 display updates
- `double_sha256()` - Optimized SHA-256d (using mbedtls)
- `count_leading_zeros()` - Hardware-optimized difficulty check

## Synchronization

### Mutexes

**stats_mutex:**
- Protects: `total_hashes`, `best_difficulty`, `nonce`, `current_hashrate`, `shares_accepted`, `shares_submitted`, `pool_connected`
- Acquired by: Mining Task (Core 0), Display Task (Core 1)
- Hold time: < 1ms (critical sections are small)

**job_mutex:**
- Protects: `current_job` structure in stratum.c
- Acquired by: Stratum Task (Core 1), Mining Task (Core 0)
- Hold time: < 1ms (job copy operation)

### Lock Ordering

To prevent deadlock, locks are always acquired in this order:
1. stats_mutex
2. job_mutex

Never acquire stats_mutex while holding job_mutex.

## Memory Layout

### Heap Usage

Approximate heap requirements:
- Stratum client: ~4KB (buffers, job storage)
- Mining context: ~2KB (SHA context, block header)
- Display: ~2KB (frame buffer)
- TCP/IP stack: ~40KB (LwIP)
- **Total:** ~50KB heap + ~32KB task stacks

### Stack Sizes

- Mining Task: 8KB (Core 0)
- Stratum Task: 8KB (Core 1)
- Display Task: 4KB (Core 1)
- Total: 20KB for application tasks

## Configuration Files

### config.h (user-created, gitignored)

```c
#define WIFI_SSID "your_wifi"
#define WIFI_PASS "your_password"
#define POOL_URL "solo.ckpool.org"
#define POOL_PORT 3333
#define WALLET_ADDRESS "your_btc_address"
#define WORKER_NAME "ESP32Miner"
```

### Conditional Compilation

```c
#ifdef WIFI_SSID
    // WiFi and Stratum code compiled
    wifi_init();
    stratum_init();
    // Create Stratum task
#else
    // Mining runs offline with mock data
    // Allows CI/CD builds without credentials
#endif
```

## Performance Characteristics

### Expected Throughput

- **Hashrate:** 20-50 H/s per core
- **Network:** ~1-10 messages/minute (Stratum)
- **Display:** Updated every 2 seconds
- **Share submissions:** Every hours/days/never (depends on difficulty)

### Latency

- **Job reception to mining:** < 10ms
- **Share found to submission:** < 50ms
- **Display update:** 2000ms interval

### CPU Utilization

- **Core 0:** ~95% mining, ~5% housekeeping
- **Core 1:** ~10% network, ~5% display, ~85% idle

## Error Handling

### Network Errors

- **Connection lost:** Automatic reconnect after 30s
- **DNS failure:** Retry with exponential backoff
- **Invalid JSON:** Log error, continue listening
- **Pool rejection:** Log, don't resubmit same share

### Mining Errors

- **Invalid job data:** Skip job, wait for next
- **Header build failure:** Log error, continue with old job
- **SHA context failure:** Fatal, restart task

### Resource Errors

- **Mutex timeout:** Skip operation, log warning
- **Heap exhaustion:** Fatal, cannot recover
- **Stack overflow:** Fatal, watchdog reset

## Future Enhancements

### Planned Features

1. **Dual-core mining:** Both cores mining with different nonce ranges
2. **Hardware SHA acceleration:** Use ESP32-S3 crypto accelerator
3. **Stratum V2:** Support for newer protocol
4. **Temperature monitoring:** Built-in temp sensor
5. **Fan control:** PWM fan speed based on temperature
6. **Web interface:** Configure via HTTP instead of config.h
7. **OTA updates:** Flash new firmware over WiFi
8. **Difficulty estimation:** Better share prediction

### Optimization Opportunities

1. **IRAM placement:** Move SHA-256 to IRAM for speed
2. **Assembly optimization:** Hand-coded Xtensa SHA-256
3. **Midstate caching:** Cache partial hash for nonce scanning
4. **Zero-copy buffers:** Reduce memory allocations
5. **Batch processing:** Process multiple nonces per loop

---

**Last Updated:** January 2026
**Architecture Version:** 1.0
**Compatible with:** ESP-IDF v5.1.2+, ESP32-S3
