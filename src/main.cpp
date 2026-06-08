/*
 * Smart Dustbin - ESP32 with Servo Motor Control
 * Features:
 * - WiFi connectivity with auto-reconnection
 * - MQTT broker connection (Non-blocking)
 * - Servo motor control via PWM
 * - Status publishing (OPENED, CLOSED, ONLINE)
 * - Serial debugging
 * - Non-blocking code using millis()
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ===================== PIN CONFIGURATION =====================
#define SERVO_PIN 19
#define SERVO_PWM_CHANNEL 0

// ===================== SERVO CONFIGURATION =====================
#define SERVO_MIN_ANGLE 0      // Closed position
#define SERVO_MAX_ANGLE 90     // Open position
#define SERVO_FREQ 50          // 50Hz for standard servo
#define SERVO_MIN_US 500       // Minimum pulse width in microseconds
#define SERVO_MAX_US 2400      // Maximum pulse width in microseconds

// ===================== WIFI CONFIGURATION =====================
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
#define WIFI_RETRY_INTERVAL 5000  // ms

// ===================== MQTT CONFIGURATION =====================
const char* mqtt_server = "turin.iotserver.uz";
const int mqtt_port = 2890;
const char* mqtt_user = "userTTPU";
const char* mqtt_pass = "mqttpass";
const char* mqtt_client_id = "ESP32_Dustbin_001";
const char* mqtt_subscribe_topic = "dustbin/control";
const char* mqtt_publish_topic = "dustbin/status";
#define MQTT_RETRY_INTERVAL 3000   // ms
#define MQTT_RECONNECT_ATTEMPTS 5

// ===================== SERVO CONTROL TIMING =====================
#define SERVO_MOVEMENT_TIME 1500   // ms - time to move servo
#define SERVO_HOLD_TIME 3000       // ms - time to hold open position

// ===================== GLOBAL VARIABLES =====================
WiFiClient espClient;
PubSubClient client(espClient);
Servo servoMotor;

// State tracking variables
enum DustbinState { IDLE, OPENING, HOLDING, CLOSING };
DustbinState currentState = IDLE;
unsigned long lastServoActionTime = 0;

// Timing variables for non-blocking operations
unsigned long lastWiFiCheckTime = 0;
unsigned long lastMqttCheckTime = 0;
unsigned long lastStatusPublishTime = 0;
#define STATUS_PUBLISH_INTERVAL 30000  // Publish online status every 30 seconds

// Connection status tracking
bool wifiConnected = false;
bool mqttConnected = false;

// ===================== FUNCTION PROTOTYPES =====================
void initializeSerial();
void initializeServo();
void connectToWiFi();
void checkWiFiConnection();
void setupMQTT();
void reconnectMQTT();
void checkMQTTConnection();
void publishStatus(const char* status);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void handleServoCommand(String command);
void updateServoState();
void logDebug(const char* message);
void logError(const char* message);

// ===================== INITIALIZATION FUNCTIONS =====================

void initializeSerial() {
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n\n");
    Serial.println("╔══════════════════════════════════════════════════════╗");
    Serial.println("║         Smart Dustbin - ESP32 Control System        ║");
    Serial.println("║                 Starting System...                  ║");
    Serial.println("╚══════════════════════════════════════════════════════╝\n");
}

void initializeServo() {
    logDebug("Initializing servo motor...");
    
    servoMotor.setPeriodHertz(SERVO_FREQ);
    if (servoMotor.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US)) {
        servoMotor.write(SERVO_MIN_ANGLE);  // Start in closed position
        logDebug("✓ Servo motor initialized successfully");
    } else {
        logError("✗ Failed to attach servo motor!");
    }
}

void setupMQTT() {
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);
    logDebug("MQTT client configured");
}

// ===================== WIFI FUNCTIONS =====================

void connectToWiFi() {
    logDebug("\n");
    Serial.print("Connecting to WiFi: ");
    Serial.print(ssid);
    Serial.println("...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    const int maxAttempts = 40;  // ~20 seconds timeout
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.print("✓ WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        wifiConnected = false;
        logError("✗ Failed to connect to WiFi");
    }
}

void checkWiFiConnection() {
    // Check WiFi status periodically
    if (millis() - lastWiFiCheckTime > WIFI_RETRY_INTERVAL) {
        lastWiFiCheckTime = millis();
        
        if (WiFi.status() == WL_CONNECTED) {
            if (!wifiConnected) {
                wifiConnected = true;
                Serial.println("✓ WiFi reconnected!");
            }
        } else {
            if (wifiConnected) {
                wifiConnected = false;
                logError("✗ WiFi connection lost!");
            }
        }
    }
}

// ===================== MQTT FUNCTIONS =====================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.print("📨 MQTT message received on topic [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);
    
    // Trim whitespace
    message.trim();
    
    // Handle commands
    if (message == "OPEN") {
        logDebug("→ Command: OPEN");
        handleServoCommand("OPEN");
    } 
    else if (message == "CLOSE") {
        logDebug("→ Command: CLOSE");
        handleServoCommand("CLOSE");
    }
    else {
        Serial.print("⚠ Unknown command: ");
        Serial.println(message);
    }
}

void reconnectMQTT() {
    if (mqttConnected || !wifiConnected) {
        return;  // Already connected or WiFi not available
    }
    
    if (millis() - lastMqttCheckTime < MQTT_RETRY_INTERVAL) {
        return;  // Not time to retry yet
    }
    
    lastMqttCheckTime = millis();
    
    Serial.print("Attempting MQTT connection... (");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.println(")");
    
    // Attempt to connect
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
        mqttConnected = true;
        Serial.println("✓ MQTT connected!");
        
        // Subscribe to control topic
        if (client.subscribe(mqtt_subscribe_topic)) {
            Serial.print("✓ Subscribed to: ");
            Serial.println(mqtt_subscribe_topic);
        } else {
            logError("✗ Failed to subscribe!");
        }
        
        // Publish online status
        publishStatus("ONLINE");
    } 
    else {
        mqttConnected = false;
        Serial.print("✗ MQTT connection failed, rc=");
        Serial.print(client.state());
        Serial.println(" (will retry)");
    }
}

void checkMQTTConnection() {
    if (mqttConnected) {
        if (!client.connected()) {
            mqttConnected = false;
            logError("✗ MQTT disconnected!");
        }
    } else if (wifiConnected) {
        reconnectMQTT();
    }
}

void publishStatus(const char* status) {
    if (!client.connected()) {
        Serial.print("⚠ Cannot publish (MQTT not connected): ");
        Serial.println(status);
        return;
    }
    
    if (client.publish(mqtt_publish_topic, status)) {
        Serial.print("📤 Status published: ");
        Serial.println(status);
    } else {
        Serial.print("✗ Failed to publish status: ");
        Serial.println(status);
    }
}

// ===================== SERVO CONTROL FUNCTIONS =====================

void handleServoCommand(String command) {
    if (command == "OPEN") {
        currentState = OPENING;
        lastServoActionTime = millis();
        logDebug("🔓 Dustbin opening...");
    } 
    else if (command == "CLOSE") {
        currentState = CLOSING;
        lastServoActionTime = millis();
        logDebug("🔒 Dustbin closing...");
    }
}

void updateServoState() {
    unsigned long elapsedTime = millis() - lastServoActionTime;
    
    switch (currentState) {
        case OPENING:
            // Move servo to open position (90 degrees)
            servoMotor.write(SERVO_MAX_ANGLE);
            
            if (elapsedTime >= SERVO_MOVEMENT_TIME) {
                currentState = HOLDING;
                publishStatus("OPENED");
                logDebug("✓ Dustbin opened successfully");
                lastServoActionTime = millis();
            }
            break;
            
        case HOLDING:
            // Keep servo at open position
            servoMotor.write(SERVO_MAX_ANGLE);
            
            if (elapsedTime >= SERVO_HOLD_TIME) {
                currentState = CLOSING;
                lastServoActionTime = millis();
            }
            break;
            
        case CLOSING:
            // Move servo back to closed position (0 degrees)
            servoMotor.write(SERVO_MIN_ANGLE);
            
            if (elapsedTime >= SERVO_MOVEMENT_TIME) {
                currentState = IDLE;
                publishStatus("CLOSED");
                logDebug("✓ Dustbin closed successfully");
            }
            break;
            
        case IDLE:
            // Servo at rest position
            servoMotor.write(SERVO_MIN_ANGLE);
            break;
    }
}

// ===================== DEBUG/LOGGING FUNCTIONS =====================

void logDebug(const char* message) {
    Serial.print("[DEBUG] ");
    Serial.println(message);
}

void logError(const char* message) {
    Serial.print("[ERROR] ");
    Serial.println(message);
}

// ===================== SETUP =====================

void setup() {
    // Initialize serial for debugging
    initializeSerial();
    
    // Initialize servo motor
    initializeServo();
    
    // Connect to WiFi
    connectToWiFi();
    
    // Setup MQTT
    setupMQTT();
    
    Serial.println("\n✓ System initialization complete!\n");
}

// ===================== MAIN LOOP (NON-BLOCKING) =====================

void loop() {
    // Check WiFi connection status
    checkWiFiConnection();
    
    // Check and maintain MQTT connection
    checkMQTTConnection();
    
    // Process MQTT messages if connected
    if (client.connected()) {
        client.loop();
    }
    
    // Update servo state machine
    updateServoState();
    
    // Periodically publish online status
    if (millis() - lastStatusPublishTime > STATUS_PUBLISH_INTERVAL) {
        lastStatusPublishTime = millis();
        if (client.connected()) {
            publishStatus("ONLINE");
        }
    }
}