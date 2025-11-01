# ESP32-S3 Dual-Core Mining Architecture

## Overview

The ESP32-S3 Bitcoin miner implements a modular dual-core architecture that separates compute-intensive mining operations from I/O operations for maximum performance and responsiveness.

## Architecture Design

### Core 0: Primary Mining Core
**Purpose**: Dedicated SHA-256 computation for maximum mining performance

**Responsibilities**:
- Execute SHA-256 double hashing loop
- Nonce incrementing and block header management
- Difficulty calculation
- Statistics updates (via mutex, every 2 seconds)

**Key Characteristics**:
- **High Priority**: Task priority 5
- **Minimal I/O**: No display, WiFi, or network operations
- **Maximum Performance**: Uninterrupted compute cycles
- **Stack Size**: 8192 bytes

**Task**: `mining_task`

### Core 1: I/O and Management Core
**Purpose**: Handle all I/O operations without interfering with mining

**Responsibilities**:
- OLED display updates (every 2 seconds)
- WiFi management and monitoring
- Future expansion:
  - MQTT message publishing
  - Web server requests
  - Temperature monitoring
  - Fan control (PWM)
  - Network time synchronization
  - Remote configuration

**Key Characteristics**:
- **Lower Priority**: Task priority 3
- **I/O Focused**: All peripheral and network operations
- **Non-blocking**: Does not interfere with mining performance
- **Stack Size**: 4096 bytes

**Task**: `display_io_task`

## Thread-Safe Communication

### Shared Statistics
The following variables are shared between cores and protected by a mutex:

```c
static uint64_t total_hashes = 0;      // Cumulative hash count
static uint32_t best_difficulty = 0;   // Best difficulty found
static uint32_t nonce = 0;             // Current nonce value
static float current_hashrate = 0.0f;  // Real-time hashrate
```

### Mutex Protection
A FreeRTOS mutex (`stats_mutex`) protects all shared statistics:

- **Mining Task (Core 0)**: Writes statistics every 2 seconds
- **Display Task (Core 1)**: Reads statistics every 2 seconds
- **Lock Duration**: Minimal (microseconds)
- **Contention**: Very low due to synchronized 2-second intervals

### Communication Pattern

```
Core 0 (Mining)                    Core 1 (Display/IO)
================                   ===================
                                   
Mine hash                          
Increment nonce                    
Check difficulty                   
  |                                
  | Every 2 seconds                
  v                                
Lock mutex                         
Update statistics                  
Unlock mutex --------\             
  |                   |            
Continue mining       \---------> Lock mutex
                                   Read statistics
                                   Unlock mutex
                                      |
                                      v
                                   Update display
                                   Handle I/O
```

## Performance Characteristics

### Hash Rate Impact
- **Single-Core (Original)**: Mining interrupted by display updates
- **Dual-Core (New)**: Mining runs continuously, display on separate core
- **Expected Improvement**: 5-10% hashrate increase due to eliminated I/O overhead

### Thermal Considerations

⚠️ **IMPORTANT**: Dual-core mining generates more heat

**Cooling Requirements**:
- **Active cooling (fan) REQUIRED** for sustained dual-core operation
- **Temperature monitoring recommended**
- **Safe operating temperature**: < 70°C
- **Throttling threshold**: 80-85°C

See [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md) for detailed cooling guidelines.

## Code Structure

### Initialization (app_main)
1. Create mutex for statistics protection
2. Initialize NVS, I2C, and display
3. Initialize WiFi (if configured)
4. Create mining task pinned to Core 0
5. Create display/IO task pinned to Core 1

### Mining Task Flow (Core 0)
```
1. Initialize block header
2. Loop:
   - Compute SHA-256 double hash
   - Increment local nonce counter
   - Check difficulty
   - Every 2 seconds:
     - Calculate hashrate
     - Lock mutex
     - Update shared statistics
     - Unlock mutex
   - Yield to watchdog every 1000 nonces
```

### Display/IO Task Flow (Core 1)
```
1. Wait for initialization
2. Loop:
   - Lock mutex
   - Read current statistics
   - Unlock mutex
   - Update OLED display
   - Handle WiFi/network operations
   - Wait 2 seconds
```

## Benefits

### 1. Performance
- Mining core dedicated to compute-intensive SHA-256
- No I/O interruptions on mining core
- Optimal cache utilization

### 2. Responsiveness
- Display updates independent of mining
- WiFi operations don't block mining
- Better user experience

### 3. Modularity
- Clear separation of concerns
- Easy to add new I/O features to Core 1
- Mining code remains clean and focused

### 4. Thread Safety
- Mutex-protected shared data
- No race conditions
- Predictable behavior

### 5. Scalability
- Core 1 can be extended with:
  - MQTT publishing
  - Web interface
  - REST API
  - Temperature/fan control
  - Remote monitoring

## Future Enhancements

### Planned Core 1 Features
- **Temperature Monitoring**: Read ESP32 internal temperature sensor
- **Fan Control**: PWM fan speed based on temperature
- **MQTT Integration**: Publish mining statistics to MQTT broker
- **Web Dashboard**: HTTP server with mining statistics
- **WiFi Auto-reconnect**: Robust WiFi management
- **OTA Updates**: Over-the-air firmware updates

### Advanced Mining Optimizations (Core 0)
- **Midstate Caching**: Pre-compute SHA-256 midstate (~30-40% speedup)
- **Assembly Optimization**: Xtensa assembly for critical loops
- **Nonce Range Splitting**: Multiple mining tasks if needed
- **IRAM Placement**: Hot code in instruction RAM

## Configuration

### Task Priorities
```c
#define MINING_TASK_PRIORITY    5    // High priority for mining
#define DISPLAY_IO_PRIORITY     3    // Lower priority for I/O
```

### Stack Sizes
```c
#define MINING_TASK_STACK       8192 // Mining task stack
#define DISPLAY_IO_STACK        4096 // Display/IO task stack
```

### Update Intervals
```c
#define STATS_UPDATE_INTERVAL   2000000  // 2 seconds in microseconds
#define DISPLAY_UPDATE_INTERVAL 2000     // 2 seconds in milliseconds
```

## Migration from Single-Core

### What Changed
1. **Mining Task**: Now runs on Core 0 instead of Core 1
2. **Display Updates**: Moved to separate task on Core 1
3. **Statistics**: Now protected by mutex
4. **Initialization**: Creates two tasks instead of one

### Backward Compatibility
- All functionality preserved
- Same hashrate calculation method
- Same display format
- WiFi configuration unchanged

### Testing
After upgrading:
1. Monitor serial output for core assignments
2. Verify display updates every 2 seconds
3. Check mining logs show consistent hashrate
4. Monitor temperature (should be higher with dual-core)
5. Ensure WiFi remains connected (if configured)

## Troubleshooting

### Display Not Updating
- Check Core 1 task is running: Look for "Display/IO task started on core 1"
- Verify mutex is created successfully
- Check I2C connections

### Mining Performance Issues
- Verify Core 0 is running mining task
- Check for excessive mutex contention in logs
- Monitor temperature (may need active cooling)

### System Crashes
- Check stack sizes are adequate
- Verify mutex is properly initialized
- Review serial logs for stack overflow warnings

## References

- [ESP32_MINING_STRATEGIES.md](ESP32_MINING_STRATEGIES.md) - Cooling and optimization strategies
- [MINING_QUICKSTART.md](MINING_QUICKSTART.md) - Quick start guide
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html) - Task management
- [ESP32-S3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf) - Hardware details
