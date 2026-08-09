#include <SoftwareSerial.h>

// Pin Definitions
#define TRIG_PIN 6
#define ECHO_PIN 7
#define LIGHT_PIN 4
#define METAL_DETECTOR_PIN 5
#define MOTOR_A1 9
#define MOTOR_A2 10
#define MOTOR_B1 11
#define MOTOR_B2 12

// Communication
SoftwareSerial espSerial(2, 3); // RX, TX (Connect to ESP32's TX, RX)

// Constants
const int OBSTACLE_DISTANCE = 20; // cm
const unsigned long COMMAND_TIMEOUT = 200; // ms
const int JOYSTICK_DEADZONE = 20;

// Global Variables
bool isAutoMode = true;
bool metalDetected = false;
unsigned long lastCommandTime = 0;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(METAL_DETECTOR_PIN, INPUT);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  espSerial.begin(115200);
  Serial.begin(115200);
  stopMotors();
}

void loop() {
  handleSerialCommands();
  checkMetalDetection();
  
  if(isAutoMode) {
    handleAutoMode();
  }
}

// Motor Control Functions
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

void moveBackward() {
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, HIGH);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, HIGH);
}

void turnRight() {
  digitalWrite(MOTOR_A1, HIGH);
  digitalWrite(MOTOR_A2, LOW);
  digitalWrite(MOTOR_B1, LOW);
  digitalWrite(MOTOR_B2, HIGH);
}

void turnLeft() {
  digitalWrite(MOTOR_A1, LOW);
  digitalWrite(MOTOR_A2, HIGH);
  digitalWrite(MOTOR_B1, HIGH);
  digitalWrite(MOTOR_B2, LOW);
}

void spin360() {
  turnRight();
  delay(1000);
  stopMotors();
}

// Sensor Functions
long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  return pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
}

void checkMetalDetection() {
  bool currentDetection = digitalRead(METAL_DETECTOR_PIN);
  if(currentDetection != metalDetected) {
    metalDetected = currentDetection;
    espSerial.print("METAL:");
    espSerial.println(metalDetected ? "1" : "0");
    if(metalDetected) handleMetalDetection();
  }
}

// Command Handling
void handleSerialCommands() {
  while(espSerial.available()) {
    String command = espSerial.readStringUntil('\n');
    command.trim();
    
    if(command.startsWith("MODE:")) {
      isAutoMode = (command.substring(5) == "AUTO");
      stopMotors();
      Serial.print("Mode changed to: ");
      Serial.println(command.substring(5));
    }
    else if(command == "TURN360" && !isAutoMode) {
      spin360();
      lastCommandTime = millis();
      Serial.println("360 turn executed");
    }
    else if(command.startsWith("JOYSTICK:") && !isAutoMode) {
      processJoystick(command);
      lastCommandTime = millis();
    }
  }

  // Auto-stop if no recent commands
  if(!isAutoMode && (millis() - lastCommandTime > COMMAND_TIMEOUT)) {
    stopMotors();
  }
}

void processJoystick(String command) {
  int commaIndex = command.indexOf(',');
  if(commaIndex == -1) return;
  
  int x = command.substring(9, commaIndex).toInt();
  int y = command.substring(commaIndex+1).toInt();

  // Vertical movement (Y-axis)
  if(y > JOYSTICK_DEADZONE) {
    moveForward();
    Serial.println("Moving FORWARD");
  } else if(y < -JOYSTICK_DEADZONE) {
    moveBackward();
    Serial.println("Moving BACKWARD");
  }
  // Horizontal movement (X-axis)
  else if(x > JOYSTICK_DEADZONE) {
    turnRight();
    Serial.println("Turning RIGHT");
  } else if(x < -JOYSTICK_DEADZONE) {
    turnLeft();
    Serial.println("Turning LEFT");
  } else {
    stopMotors();
    Serial.println("Stopped");
  }
}

// Auto Mode Handler
void handleAutoMode() {
  if(metalDetected) {
    stopMotors();
    return;
  }

  long distance = measureDistance();
  if(distance > OBSTACLE_DISTANCE) {
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

// Metal Detection Handler
void handleMetalDetection() {
  Serial.println("Metal detected!");
  stopMotors();
  unsigned long startTime = millis();
  while(millis() - startTime < 10000) {
    digitalWrite(LIGHT_PIN, HIGH);
    delay(500);
    digitalWrite(LIGHT_PIN, LOW);
    delay(500);
    
    if(!digitalRead(METAL_DETECTOR_PIN)) {
      metalDetected = false;
      espSerial.println("METAL:0");
      break;
    }
  }
}