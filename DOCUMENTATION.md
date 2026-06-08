# Smart Dustbin - Complete Project Documentation

## 📋 Table of Contents

1. [Project Overview](#project-overview)
2. [Architecture](#architecture)
3. [Features](#features)
4. [How It Works](#how-it-works)
5. [Code Structure](#code-structure)
6. [API Reference](#api-reference)

---

## 🎯 Project Overview

The **Smart Dustbin** is an IoT-based waste management system that uses an ESP32 microcontroller to remotely control a servo motor lid via MQTT protocol. The system provides:

- **Remote Control**: Open/close lid via MQTT commands
- **Status Monitoring**: Real-time status updates (ONLINE, OPENED, CLOSED)
- **Autonomous Operation**: Auto-closes lid after opening
- **Connectivity**: WiFi + MQTT with automatic reconnection
- **Reliability**: Non-blocking code with proper error handling

---

## 🏗️ Architecture

### System Components

```
┌─────────────────────────────────────────────────────┐
│                    MQTT Broker                      │
│            (turin.iotserver.uz:2890)               │
└────────────────────┬────────────────────────────────┘
                     │ WiFi Network
                     │
        ┌────────────▼───────────┐
        │   ESP32 Microcontroller│
        │  ┌──────────────────┐  │
        │  │  Main Program    │  │
        │  │  - WiFi Manager  │  │
        │  │  - MQTT Client   │  │
        │  │  - Servo Control │  │
        │  └──────────────────┘  │
        └─────────┬───┬──────────┘
                  │   │
      ┌───────────┘   └─────────────────┐
      │                                 │
      ▼                                 ▼
  [Servo Motor]                    [Serial Monitor]
  (GPIO 19)                        (Debug Output)
      │
      ▼
  [Dustbin Lid]
```

### Data Flow

```
MQTT Topic: dustbin/control
     │
     ▼
  Parse Command
     │
     ├─→ "OPEN"  ─→ setState(OPENING)
     │
     └─→ "CLOSE" ─→ setState(CLOSING)
     │
     ▼
State Machine Update
     │
     ├─→ OPENING  → Move servo 0° → 90°
     ├─→ HOLDING  → Keep servo at 90°
     ├─→ CLOSING  → Move servo 90° → 0°
     └─→ IDLE     → Rest state
     │
     ▼
Publish Status
     │
     └─→ dustbin/status: "OPENED" / "CLOSED" / "ONLINE"
```

---

## ✨ Features

### 1. **Non-Blocking Code**
- Uses `millis()` instead of `delay()`
- Allows multiple operations to run concurrently
- System remains responsive at all times

### 2. **WiFi Management**
- Automatic WiFi connection on startup
- Periodic WiFi status checking
- Automatic reconnection if connection drops

### 3. **MQTT Communication**
- Secure connection with username/password
- Automatic MQTT reconnection
- Graceful handling of connection failures
- Periodic heartbeat (ONLINE status every 30 seconds)

### 4. **Servo Control**
- Smooth servo movement
- Configurable angles and timings
- State machine-based control
- Auto-close after opening

### 5. **Debugging & Logging**
- Serial output with visual indicators
- Detailed state transitions
- Error messages and warnings
- Connection status updates

### 6. **Error Handling**
- Graceful degradation
- Automatic retry mechanisms
- State recovery
- No blocking operations

---

## 🔄 How It Works

### System Initialization

```
1. Serial Setup (115200 baud)
   ↓
2. Servo Motor Initialization
   ├─ Set frequency to 50Hz
   ├─ Attach to GPIO 19
   └─ Initialize to closed (0°)
   ↓
3. WiFi Connection
   ├─ Set mode to STA (station)
   ├─ Connect to SSID with password
   ├─ Wait for IP assignment
   └─ Log IP address
   ↓
4. MQTT Setup
   ├─ Configure broker address
   ├─ Set callback function
   └─ Ready to connect
```

### Main Loop Operation (Non-Blocking)

The main loop runs **continuously** and performs these checks:

```cpp
loop() {
    1. checkWiFiConnection()           // ~every 5 seconds
    2. checkMQTTConnection()           // ~every 3 seconds
    3. client.loop()                   // Process MQTT messages
    4. updateServoState()              // Update servo position
    5. publishStatus() if needed       // ~every 30 seconds
}
```

This means:
- If servo is moving, it continues moving
- While servo moves, WiFi is checked
- While WiFi is checked, MQTT messages are processed
- No blocking operations prevent responsiveness

### Servo State Machine

```
Initial State: IDLE (servo at 0°)

When OPEN command received:
    IDLE → OPENING (1.5 sec) → HOLDING (3 sec) → CLOSING (1.5 sec) → IDLE

When CLOSE command received:
    → CLOSING (1.5 sec) → IDLE

States:
- IDLE:    Servo at rest (0°)
- OPENING: Servo moving to open (0° → 90°)
- HOLDING: Servo holding open (90°)
- CLOSING: Servo moving to close (90° → 0°)
```

### MQTT Command Flow

```
Receive "OPEN" on "dustbin/control"
    ↓
Parse and validate command
    ↓
Set servo state to OPENING
    ↓
[Non-blocking servo movement begins]
    ↓
After 1.5 seconds: Switch to HOLDING state
    ↓
After 3 more seconds: Switch to CLOSING state
    ↓
After 1.5 seconds: Switch to IDLE state
    ↓
Publish "OPENED" to "dustbin/status"
    ↓
Auto-close sequence complete
```

---

## 📂 Code Structure

### Main Components

#### 1. **Initialization Functions**
```cpp
initializeSerial()    // Setup serial communication
initializeServo()     // Configure and attach servo
connectToWiFi()       // Connect to WiFi network
setupMQTT()          // Configure MQTT client
```

#### 2. **WiFi Management**
```cpp
checkWiFiConnection()  // Monitor WiFi status
                       // Automatically reconnect if needed
```

#### 3. **MQTT Management**
```cpp
reconnectMQTT()       // Attempt MQTT connection with retries
checkMQTTConnection() // Monitor connection status
mqttCallback()        // Handle incoming messages
publishStatus()       // Publish status messages
```

#### 4. **Servo Control**
```cpp
handleServoCommand()  // Parse and execute commands
updateServoState()    // Update servo position (state machine)
```

#### 5. **Logging**
```cpp
logDebug()            // Print debug messages
logError()            // Print error messages
```

---

## 🔗 API Reference

### MQTT Topics

#### Subscribe To: `dustbin/control`

Receive commands to control the dustbin.

| Command | Action | Result |
|---------|--------|--------|
| `OPEN` | Open dustbin lid | Servo moves to 90°, holds 3 sec, closes automatically |
| `CLOSE` | Close dustbin lid | Servo moves to 0° immediately |

**Example MQTT Messages:**
```
Topic:   dustbin/control
Payload: OPEN

Topic:   dustbin/control
Payload: CLOSE
```

#### Publish To: `dustbin/status`

The system publishes status messages.

| Status | Meaning | Frequency |
|--------|---------|-----------|
| `ONLINE` | System is connected and operating | Every 30 seconds |
| `OPENED` | Dustbin lid is in open position | After OPEN command sequence |
| `CLOSED` | Dustbin lid is in closed position | After CLOSE command sequence |

**Example MQTT Messages:**
```
Topic:   dustbin/status
Payload: ONLINE

Topic:   dustbin/status
Payload: OPENED

Topic:   dustbin/status
Payload: CLOSED
```

---

## ⚙️ Configuration Parameters

All configurable values are defined in [include/config.h](include/config.h):

### Hardware
```cpp
#define SERVO_PIN 19           // GPIO pin for servo
#define SERVO_FREQ 50          // Servo frequency (Hz)
#define SERVO_MIN_ANGLE 0      // Closed position
#define SERVO_MAX_ANGLE 90     // Open position
```

### Timing
```cpp
#define SERVO_MOVEMENT_TIME 1500      // Time to move servo (ms)
#define SERVO_HOLD_TIME 3000          // Hold open time (ms)
#define STATUS_PUBLISH_INTERVAL 30000 // Status update interval (ms)
```

### MQTT
```cpp
#define MQTT_SERVER "turin.iotserver.uz"
#define MQTT_PORT 2890
#define MQTT_USER "userTTPU"
#define MQTT_PASS "mqttpass"
```

---

## 🚀 Performance Specifications

| Parameter | Value |
|-----------|-------|
| WiFi Reconnection Check | Every 5 seconds |
| MQTT Reconnection Attempt | Every 3 seconds |
| Status Publishing | Every 30 seconds |
| Servo Movement Time | 1.5 seconds |
| Servo Hold Time | 3 seconds |
| Serial Baud Rate | 115200 bps |
| Loop Frequency | ~100+ times/second |

---

## 🔐 Security Notes

1. **Change WiFi Credentials**: Update SSID and password before deployment
2. **MQTT Username/Password**: Currently configured; consider changing for production
3. **MQTT Over TLS**: Current implementation uses plain TCP; consider adding TLS for production
4. **Command Validation**: Only accepts "OPEN" and "CLOSE" commands

---

## 📋 Serial Output Reference

### Successful Startup
```
[DEBUG] Initializing servo motor...
✓ Servo motor initialized successfully
Connecting to WiFi: YOUR_WIFI...
✓ WiFi connected! IP: 192.168.1.100
MQTT client configured
✓ System initialization complete!
Attempting MQTT connection... (turin.iotserver.uz:2890)
✓ MQTT connected!
✓ Subscribed to: dustbin/control
📤 Status published: ONLINE
```

### Processing Command
```
📨 MQTT message received on topic [dustbin/control]: OPEN
[DEBUG] → Command: OPEN
[DEBUG] 🔓 Dustbin opening...
[DEBUG] ✓ Dustbin opened successfully
📤 Status published: OPENED
```

### Connection Recovery
```
✗ WiFi connection lost!
✗ WiFi connected!
✗ MQTT disconnected!
Attempting MQTT connection... (turin.iotserver.uz:2890)
✓ MQTT connected!
✓ Subscribed to: dustbin/control
```

---

## 📚 Further Development

### Potential Enhancements

1. **Telemetry**: Add distance sensors to detect lid position
2. **Notifications**: Send alerts if lid is stuck
3. **Scheduling**: Automatic open/close on schedule
4. **Statistics**: Track open/close counts
5. **OTA Updates**: Remote firmware updates
6. **Security**: Add authentication and encryption
7. **Web Dashboard**: Create a web interface for monitoring
8. **Temperature Sensor**: Monitor internal temperature
9. **Motion Detection**: Detect unauthorized access
10. **Battery Backup**: Add UPS for emergency operation

---

## 📞 Support & Troubleshooting

Refer to [SETUP_GUIDE.md](SETUP_GUIDE.md#-troubleshooting) for common issues and solutions.

---

**Last Updated:** 2026-06-04
**Version:** 1.0.0
**Status:** Production Ready ✓
