# Smart Dustbin - ESP32 Setup Guide

## 📋 System Overview

This is a complete IoT Smart Dustbin system using:
- **ESP32 Microcontroller** - Main processor
- **Servo Motor** - Lid control mechanism
- **WiFi** - Network connectivity
- **MQTT** - Remote command communication
- **Non-blocking Code** - Efficient resource usage

## 🔧 Hardware Requirements

- ESP32 DevKit V1 (or compatible)
- MG996R/SG90 Servo Motor
- USB Cable (for programming)
- Power supply (5V for servo)
- Jumper wires

## 🔌 Wiring Diagram

```
ESP32 Pin 19 -----> Servo Motor PWM Signal
ESP32 GND --------> Servo Motor GND
5V Power --------> Servo Motor VCC
```

## 📝 Configuration

### 1. WiFi Credentials
Edit [src/main.cpp](src/main.cpp#L35-L37):

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

Replace with your actual WiFi network details.

### 2. MQTT Broker Settings
Current settings (already configured):
- **Host:** `turin.iotserver.uz`
- **Port:** `2890`
- **Username:** `userTTPU`
- **Password:** `mqttpass`
- **Subscribe Topic:** `dustbin/control`
- **Publish Topic:** `dustbin/status`

## 🚀 Installation & Upload

### Using PlatformIO (VS Code)

1. **Install PlatformIO Extension** in VS Code (if not already installed)

2. **Build the project:**
   ```bash
   platformio run -e esp32dev
   ```

3. **Upload to ESP32:**
   ```bash
   platformio run --target upload -e esp32dev
   ```

4. **Monitor Serial Output:**
   ```bash
   platformio device monitor
   ```

### Using Arduino IDE

1. Install ESP32 Board Support:
   - File → Preferences → Add this to Additional Boards Manager URLs:
   - `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board Manager → Search "ESP32" → Install

2. Install Libraries:
   - Sketch → Include Library → Manage Libraries
   - Install: "ESP32Servo" by Kevin Harrington
   - Install: "PubSubClient" by Nick O'Leary

3. Configure Board Settings:
   - Tools → Board → ESP32 Dev Module
   - Tools → Port → Select your COM port

4. Upload the code

## 📡 MQTT Commands

### Subscribe Topic: `dustbin/control`

Send commands to control the dustbin:

```
OPEN  → Opens the lid (moves servo to 90°, holds for 3 seconds, closes automatically)
CLOSE → Closes the lid (moves servo to 0°)
```

### Publish Topic: `dustbin/status`

The system publishes:

```
ONLINE  → System is online (every 30 seconds)
OPENED  → Lid successfully opened
CLOSED  → Lid successfully closed
```

## 🧪 Testing the System

### Using MQTT Client (e.g., MQTT Explorer)

1. Connect to MQTT Broker:
   - Broker: `turin.iotserver.uz:2890`
   - Username: `userTTPU`
   - Password: `mqttpass`

2. Monitor publications:
   - Subscribe to `dustbin/status`
   - You should see "ONLINE" messages every 30 seconds

3. Send commands:
   - Topic: `dustbin/control`
   - Payload: `OPEN` or `CLOSE`

### Using Command Line (with mosquitto-clients)

Monitor status:
```bash
mosquitto_sub -h turin.iotserver.uz -p 2890 \
  -u userTTPU -P mqttpass \
  -t "dustbin/status"
```

Send command:
```bash
mosquitto_pub -h turin.iotserver.uz -p 2890 \
  -u userTTPU -P mqttpass \
  -t "dustbin/control" \
  -m "OPEN"
```

## 🔍 Serial Monitor Output

When the system starts, you'll see:

```
╔══════════════════════════════════════════════════════╗
║         Smart Dustbin - ESP32 Control System        ║
║                 Starting System...                  ║
╚══════════════════════════════════════════════════════╝

[DEBUG] Initializing servo motor...
✓ Servo motor initialized successfully
Connecting to WiFi: YOUR_WIFI...
✓ WiFi connected! IP: 192.168.X.X
MQTT client configured
✓ System initialization complete!

Attempting MQTT connection... (turin.iotserver.uz:2890)
✓ MQTT connected!
✓ Subscribed to: dustbin/control
📤 Status published: ONLINE
```

## 🔄 System State Flow

```
IDLE
  ↓
OPENING (1.5 sec) → moves servo 0° → 90°
  ↓
HOLDING (3 sec) → keeps servo at 90°
  ↓
CLOSING (1.5 sec) → moves servo 90° → 0°
  ↓
IDLE
```

## ⚡ Features Implemented

✅ Non-blocking code (uses `millis()` instead of `delay()`)
✅ Automatic WiFi reconnection
✅ Automatic MQTT reconnection
✅ State machine for servo control
✅ Status publishing (ONLINE/OPENED/CLOSED)
✅ Serial debugging with visual indicators
✅ Error handling and logging
✅ Periodic status updates (every 30 seconds)
✅ Clean code structure with comments

## 🐛 Troubleshooting

### "ESP32Servo.h not found"
- Run: `platformio lib install`
- Check [platformio.ini](platformio.ini) has the library dependency

### WiFi not connecting
- Verify SSID and password in [src/main.cpp](src/main.cpp#L35-L37)
- Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Move closer to router

### MQTT not connecting
- Verify broker address and port
- Check username and password
- Ensure MQTT broker is online: `ping turin.iotserver.uz`

### Servo not moving
- Check GPIO 19 connection
- Verify servo power supply (5V)
- Check servo signal cable polarity

### Serial monitor shows garbage
- Set baud rate to 115200 in Serial Monitor

## 📊 Power Consumption

- ESP32 idle: ~80-120 mA
- WiFi active: ~150-200 mA
- Servo active: ~500-1000 mA
- **Total:** ~1-2A (with servo)

## 📝 License

This project is open source. Feel free to modify and use as needed.

## 📚 References

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PubSubClient Library](https://github.com/knolleary/pubsub)
- [ESP32Servo Library](https://github.com/madhephaestus/ESP32Servo)
