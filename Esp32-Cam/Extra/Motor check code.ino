
#include <Arduino.h>
#include <ESP32Servo.h>

// Ultrasonic sensor pins
#define TRIG_PIN 12
#define ECHO_PIN 13
const float TRIGGER_DISTANCE = 50.0; // Distance in cm to trigger servo
const unsigned long DEBOUNCE_DELAY = 2000; // 2 seconds debounce

// Servo pin
#define SERVO_PIN 14

// Servo object
Servo servo;
bool servoPosition = false; // false = 0 degrees, true = 90 degrees
unsigned long lastTriggerTime = 0;

// Function to measure distance using ultrasonic sensor
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2; // Distance in cm
  return distance;
}

void setup() {
  Serial.begin(115200);

  // Initialize ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize servo
  servo.attach(SERVO_PIN);
  servo.write(0); // Start at 0 degrees
  Serial.println("Servo initialized");
}

void loop() {
  // Check ultrasonic sensor
  float distance = getDistance();
  if (distance > 0 && distance < TRIGGER_DISTANCE && (millis() - lastTriggerTime) > DEBOUNCE_DELAY) {
    Serial.print("Object detected at ");
    Serial.print(distance);
    Serial.println(" cm");
    lastTriggerTime = millis();

    // Toggle servo position
    servoPosition = !servoPosition;
    if (servoPosition) {
      servo.write(90); // Rotate to 90 degrees
      Serial.println("Servo rotated to 90 degrees");
    } else {
      servo.write(0);  // Rotate back to 0 degrees
      Serial.println("Servo rotated to 0 degrees");
    }
  }
}
