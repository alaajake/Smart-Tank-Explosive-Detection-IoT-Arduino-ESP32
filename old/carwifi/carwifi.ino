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
SoftwareSerial espSerial(2, 3); // RX, TX (Connect to ESP32's TX/RX)

// Constants
const int OBSTACLE_DISTANCE = 20; // cm
const unsigned long COMMAND_TIMEOUT = 200; // ms
const unsigned long METAL_UPDATE_INTERVAL = 1000; // ms

// Global Variables
bool isAutoMode = true;
bool metalDetected = false;
unsigned long lastCommandTime = 0;
unsigned long lastMetalUpdate = 0;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(METAL_DETECTOR_PIN, INPUT);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);

  espSerial.begin(9600);
  Serial.begin(9600);
  stopMotors();
}

void loop() {
  handleESPCommands();
  checkMetalDetection();
  
  if(isAutoMode) {
    handleAutoMode();
  } else {
    handleManualMode();
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
  delay(1000); // Adjust this delay based on your motors' speed
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
    
    if(metalDetected) {
      handleMetalDetection();
    }
  }
}

// Mode Handlers
void handleAutoMode() {
  long distance = measureDistance();
  
  if(metalDetected) {
    stopMotors();
    return;
  }

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

void handleManualMode() {
  if(millis() - lastCommandTime > COMMAND_TIMEOUT) {
    stopMotors();
  }
}

// Communication Handler
void handleESPCommands() {
  if(espSerial.available()) {
    String command = espSerial.readStringUntil('\n');
    command.trim();
    
    if(command.startsWith("MODE:")) {
      isAutoMode = (command.substring(5) == "AUTO");
      stopMotors();
    }
    else if(command == "TURN360") {
      spin360();
    }
    else if(command.startsWith("MOVE:")) {
      String direction = command.substring(5);
      lastCommandTime = millis();
      
      if(direction == "FORWARD") moveForward();
      else if(direction == "BACKWARD") moveBackward();
      else if(direction == "LEFT") turnLeft();
      else if(direction == "RIGHT") turnRight();
      else if(direction == "STOP") stopMotors();
    }
  }
}

// Metal Detection Handler
void handleMetalDetection() {
  unsigned long startTime = millis();
  while(millis() - startTime < 10000) { // 10-second alert
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