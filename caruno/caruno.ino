#include <Arduino.h>
//#include <SoftwareSerial.h>

// Pin definitions
#define TRIG_PIN 6
#define ECHO_PIN 7
#define LIGHT_PIN 4
#define METAL_DETECTOR_PIN 3
#define MOTOR_A1 9
#define MOTOR_A2 10
#define MOTOR_B1 11
#define MOTOR_B2 12

// SoftwareSerial setup
//SoftwareSerial espSerial(10, 11); // RX, TX

// Constants
const int obstacleDistance = 20; // Distance in cm to detect obstacles

// Variables
bool isAutoMode = true;
bool metalDetected = false;

// Function to measure distance using ultrasonic sensor
long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

// Motor control functions
void stopMotors() {
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, LOW);
}

void moveForward() {
  digitalWrite(MOTOR_A1, HIGH);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, HIGH);
  digitalWrite(MOTOR_B2, LOW);
}

void turnRight() {
  digitalWrite(MOTOR_A1, HIGH);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, HIGH);
}

void handleAutoMode() {
  long distance = measureDistance();
  metalDetected = digitalRead(METAL_DETECTOR_PIN);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Metal detected: ");
  Serial.println(metalDetected);

  if (metalDetected) {
    stopMotors();
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) { // Flash for 10 seconds
      digitalWrite(LIGHT_PIN, HIGH);
      delay(500);
      digitalWrite(LIGHT_PIN, LOW);
      delay(500);
      if (digitalRead(METAL_DETECTOR_PIN) == 0) break; // Stop flashing if metal is no longer detected
    }
    return;
  }

  if (distance > obstacleDistance) {
    moveForward();
    digitalWrite(LIGHT_PIN, HIGH);
  } else {
    stopMotors();
    digitalWrite(LIGHT_PIN, LOW);
    delay(500);
    turnRight();
    delay(500);
  }
}

void setup() {
  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(METAL_DETECTOR_PIN, INPUT);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  // Start SoftwareSerial
  //espSerial.begin(9600);
  // Start hardware serial for debugging
  Serial.begin(9600);
  stopMotors();
}

void loop() {
  handleAutoMode();
}
