/*
 * Configuration Header for Smart Dustbin
 * 
 * This file contains all configurable parameters
 * Modify these values to customize your system
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== HARDWARE CONFIGURATION ====================

// Servo Pin Configuration (GPIO)
#define SERVO_PIN 19
#define SERVO_PWM_CHANNEL 0
#define SERVO_FREQ 50              // Hz (standard servo frequency)
#define SERVO_MIN_US 500           // Minimum pulse width (microseconds)
#define SERVO_MAX_US 2400          // Maximum pulse width (microseconds)

// Servo Angles
#define SERVO_MIN_ANGLE 0          // Closed position (degrees)
#define SERVO_MAX_ANGLE 90         // Open position (degrees)

// ==================== TIMING CONFIGURATION ====================

// Servo Movement Timing
#define SERVO_MOVEMENT_TIME 1500   // Time to move servo (milliseconds)
#define SERVO_HOLD_TIME 3000       // Time to hold open position (ms)

// Connection Check Intervals
#define WIFI_RETRY_INTERVAL 5000   // Check WiFi every 5 seconds
#define MQTT_RETRY_INTERVAL 3000   // Try MQTT reconnect every 3 seconds
#define STATUS_PUBLISH_INTERVAL 30000  // Publish status every 30 seconds

// ==================== MQTT CONFIGURATION ====================

// MQTT Broker Settings
#define MQTT_SERVER "turin.iotserver.uz"
#define MQTT_PORT 2890
#define MQTT_USER "userTTPU"
#define MQTT_PASS "mqttpass"
#define MQTT_CLIENT_ID "ESP32_Dustbin_001"

// MQTT Topics
#define MQTT_SUBSCRIBE_TOPIC "dustbin/control"
#define MQTT_PUBLISH_TOPIC "dustbin/status"

// ==================== COMMAND STRINGS ====================

#define CMD_OPEN "OPEN"
#define CMD_CLOSE "CLOSE"
#define STATUS_ONLINE "ONLINE"
#define STATUS_OPENED "OPENED"
#define STATUS_CLOSED "CLOSED"

#endif
