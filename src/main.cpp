#include <Arduino.h>
#include <ESP32Servo.h>

// HC-SR04 pins
#define TRIGGER_PIN  5
#define ECHO_PIN     18

// Servo pin
#define SERVO_PIN    19

// Distance limit
#define MAX_DISTANCE 20

Servo servoMotor;

long readDistance() {

  long duration;
  long distance;

  // Clear trigger
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  // Send pulse
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Read echo
  duration = pulseIn(ECHO_PIN, HIGH);

  // Convert to centimeters
  distance = duration * 0.034 / 2;

  return distance;
}

void setup() {

  Serial.begin(115200);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // ESP32 servo setup
  servoMotor.setPeriodHertz(50);
  servoMotor.attach(SERVO_PIN, 500, 2400);

  // Start closed
  servoMotor.write(0);

  Serial.println("ESP32 Smart Dustbin Started");
}

void loop() {

  long distance = readDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Object detected
  if (distance > 0 && distance <= MAX_DISTANCE) {

    Serial.println("OPEN LID");

    servoMotor.write(90);
    delay(3000);

  } else {

    Serial.println("CLOSE LID");

    servoMotor.write(0);
  }

  delay(500);
}
