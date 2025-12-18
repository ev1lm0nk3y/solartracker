/*
  Solar Tracker Code - Version 2
  
  This sketch implements a "follow the sun" routine for a dual-axis solar tracker.
  It uses four light sensors (LDRs) to determine the sun's position and controls
  two motors (for rotation and pitch) to keep a solar panel aimed at the sun.

  New features in V2:
  - I2C 20x4 LCD for diagnostics
  - GY-271 (QMC5883L) compass for heading
  - Startup routine to check components and home motors
  - Shutdown routine to flatten the panel in low light

  Hardware requirements:
  - Arduino Micro or compatible
  - Arduino Motor Shield Rev3 or compatible
  - 4 Light Dependent Resistors (LDRs)
  - Rotational motor (Motor A)
  - Pitch motor (Motor B)
  - 20x4 I2C LCD Display
  - GY-271 (QMC5883L) Compass
*/

// --- Libraries ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <QMC5883L.h>

// --- Pin Definitions ---

// LDRs are connected to analog pins
const int LDR_TOP_LEFT_PIN = A0;
const int LDR_TOP_RIGHT_PIN = A1;
const int LDR_BOTTOM_LEFT_PIN = A2;
const int LDR_BOTTOM_RIGHT_PIN = A3;

// Motor Shield Rev3 Pin Definitions
// Motor A (Rotation)
const int MOTOR_A_DIR_PIN = 12;
const int MOTOR_A_PWM_PIN = 3;
const int MOTOR_A_BRAKE_PIN = 9;

// Motor B (Pitch)
const int MOTOR_B_DIR_PIN = 13;
const int MOTOR_B_PWM_PIN = 11;
const int MOTOR_B_BRAKE_PIN = 8;

// --- Constants ---

const int MOTOR_SPEED = 200; // PWM value for motor speed (0-255)
const int TOLERANCE = 50;    // Sensor difference tolerance to prevent jitter
const int SHUTDOWN_LIGHT_THRESHOLD = 150; // Average light level to trigger shutdown
const long LCD_UPDATE_INTERVAL = 1000; // Update LCD every 1000ms
const int HEADING_LIMIT_MIN = 80;   // Minimum angle (e.g., East limit)
const int HEADING_LIMIT_MAX = 280;  // Maximum angle (e.g., West limit)

// --- Objects ---

// LCD: Address 0x27, 20 columns, 4 rows
LiquidCrystal_I2C lcd(0x27, 20, 4); 
QMC5883L compass;

// --- Global Variables ---

// Sensor values
int ldrTopLeft, ldrTopRight, ldrBottomLeft, ldrBottomRight;
int avgTop, avgBottom, avgLeft, avgRight, avgLight;
int verticalDiff, horizontalDiff;

// State variables
bool isShutdown = false;
String pitchStatus = "STOP";
String rotationStatus = "STOP";
int heading = 0;

// Timing variables
unsigned long lastLcdUpdateTime = 0;


void setup() {
  Serial.begin(9600);
  
  // Initialize Motor Pins
  pinMode(MOTOR_A_DIR_PIN, OUTPUT);
  pinMode(MOTOR_A_PWM_PIN, OUTPUT);
  pinMode(MOTOR_A_BRAKE_PIN, OUTPUT);
  digitalWrite(MOTOR_A_BRAKE_PIN, LOW); // Disable brake

  pinMode(MOTOR_B_DIR_PIN, OUTPUT);
  pinMode(MOTOR_B_PWM_PIN, OUTPUT);
  pinMode(MOTOR_B_BRAKE_PIN, OUTPUT);
  digitalWrite(MOTOR_B_BRAKE_PIN, LOW); // Disable brake

  startupRoutine();
}

void loop() {
  readSensors();
  readCompass(); // Update heading for limit checks
  
  avgLight = (ldrTopLeft + ldrTopRight + ldrBottomLeft + ldrBottomRight) / 4;

  // Check for shutdown condition
  if (avgLight < SHUTDOWN_LIGHT_THRESHOLD && !isShutdown) {
    shutdownRoutine();
  } else if (avgLight >= SHUTDOWN_LIGHT_THRESHOLD) {
    if(isShutdown) {
      // Waking up from shutdown
      isShutdown = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Waking up...");
      delay(2000);
    }
    
    // --- Sun Tracking Logic ---
    avgTop = (ldrTopLeft + ldrTopRight) / 2;
    avgBottom = (ldrBottomLeft + ldrBottomRight) / 2;
    avgLeft = (ldrTopLeft + ldrBottomLeft) / 2;
    avgRight = (ldrTopRight + ldrBottomRight) / 2;

    verticalDiff = avgTop - avgBottom;
    horizontalDiff = avgLeft - avgRight;

    // Pitch control
    if (abs(verticalDiff) > TOLERANCE) {
      if (verticalDiff > 0) pitchDown();
      else pitchUp();
    } else {
      stopPitch();
    }

    // Rotation control
    if (abs(horizontalDiff) > TOLERANCE) {
      if (horizontalDiff > 0) rotateCounterClockwise();
      else rotateClockwise();
    } else {
      stopRotation();
    }
  }

  // Periodically update diagnostics
  if (millis() - lastLcdUpdateTime > LCD_UPDATE_INTERVAL) {
    updateLCD();
    lastLcdUpdateTime = millis();
  }
  
  delay(100); // Main loop delay
}

// --- Primary Routines ---

void startupRoutine() {
  // Initialize I2C and LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Starting...");
  Serial.println("System Starting...");

  // Initialize Compass
  lcd.setCursor(0, 1);
  compass.init();
  // NOTE: You may need to run a calibration sketch for your specific compass
  // compass.setCalibration(-1940, 1968, -2048, 1826, -2396, 2045);
  lcd.print("Compass Initialized");
  Serial.println("Compass Initialized");
  
  delay(2000);
  lcd.clear();
  lcd.print("Homing motors...");
  Serial.println("Homing motors...");

  // --- Home Motors ---
  // IMPORTANT: Implement homing logic, preferably with limit switches.
  // This is a placeholder to move motors to a known start position.
  homePitchMotor();
  homeRotationMotor();
  
  lcd.clear();
  lcd.print("Startup Complete.");
  Serial.println("Startup Complete.");
  delay(1000);
}

void shutdownRoutine() {
  isShutdown = true;
  Serial.println("Low light detected. Shutting down.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Low light detected.");
  lcd.setCursor(0, 1);
  lcd.print("Shutting down...");
  
  stopRotation();
  
  // --- Flatten Panel ---
  // IMPORTANT: Implement logic to move pitch motor to the flat position.
  // This is a placeholder. A limit switch is recommended.
  flattenPitchMotor();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("System asleep.");
  Serial.println("System asleep.");
}

// --- Sensor & Display Functions ---

void readSensors() {
  ldrTopLeft = analogRead(LDR_TOP_LEFT_PIN);
  ldrTopRight = analogRead(LDR_TOP_RIGHT_PIN);
  ldrBottomLeft = analogRead(LDR_BOTTOM_LEFT_PIN);
  ldrBottomRight = analogRead(LDR_BOTTOM_RIGHT_PIN);
}

void readCompass() {
  compass.read();
  heading = compass.getAzimuth();
}

void updateLCD() {
  if(isShutdown) return; // Don't update LCD if sleeping
  
  lcd.clear();
  char buffer[21];
  
  // Line 0 & 1: LDR values
  sprintf(buffer, "TL:%-4d TR:%-4d", ldrTopLeft, ldrTopRight);
  lcd.setCursor(0,0);
  lcd.print(buffer);
  sprintf(buffer, "BL:%-4d BR:%-4d", ldrBottomLeft, ldrBottomRight);
  lcd.setCursor(0,1);
  lcd.print(buffer);

  // Line 2: Motor Status
  sprintf(buffer, "Pitch:%-4s Rot:%-4s", pitchStatus.c_str(), rotationStatus.c_str());
  lcd.setCursor(0,2);
  lcd.print(buffer);

  // Line 3: Heading
  sprintf(buffer, "Heading: %d deg", heading);
  lcd.setCursor(0,3);
  lcd.print(buffer);

  // Also print to Serial for debugging
  Serial.print("Pitch: " + pitchStatus);
  Serial.print(" | Rotation: " + rotationStatus);
  Serial.print(" | Heading: " + String(heading));
  Serial.println(" | Avg Light: " + String(avgLight));
}


// --- Motor Control Functions ---

void pitchUp() {
  digitalWrite(MOTOR_B_DIR_PIN, HIGH);
  analogWrite(MOTOR_B_PWM_PIN, MOTOR_SPEED);
  pitchStatus = "UP";
}

void pitchDown() {
  digitalWrite(MOTOR_B_DIR_PIN, LOW);
  analogWrite(MOTOR_B_PWM_PIN, MOTOR_SPEED);
  pitchStatus = "DOWN";
}

void stopPitch() {
  analogWrite(MOTOR_B_PWM_PIN, 0);
  pitchStatus = "STOP";
}

void rotateClockwise() {
  if (heading >= HEADING_LIMIT_MAX) {
    stopRotation();
    rotationStatus = "MAX";
    return;
  }
  digitalWrite(MOTOR_A_DIR_PIN, HIGH);
  analogWrite(MOTOR_A_PWM_PIN, MOTOR_SPEED);
  rotationStatus = "CW";
}

void rotateCounterClockwise() {
  if (heading <= HEADING_LIMIT_MIN) {
    stopRotation();
    rotationStatus = "MIN";
    return;
  }
  digitalWrite(MOTOR_A_DIR_PIN, LOW);
  analogWrite(MOTOR_A_PWM_PIN, MOTOR_SPEED);
  rotationStatus = "CCW";
}

void stopRotation() {
  analogWrite(MOTOR_A_PWM_PIN, 0);
  rotationStatus = "STOP";
}

void homePitchMotor(){
  // Placeholder: Move motor for a fixed duration.
  // Replace with limit switch logic for accurate homing.
  Serial.println("Homing Pitch (placeholder)...");
  pitchUp();
  delay(5000); // Adjust this duration based on your hardware
  stopPitch();
}

void flattenPitchMotor(){
  // Placeholder: Move motor for a fixed duration.
  // Replace with limit switch logic for accurate flattening.
  Serial.println("Flattening Pitch (placeholder)...");
  pitchDown();
  delay(5000); // Adjust this duration based on your hardware
  stopPitch();
}

void homeRotationMotor(){
  // Placeholder: Move motor for a fixed duration.
  // Replace with limit switch logic or compass-based homing.
  Serial.println("Homing Rotation (placeholder)...");
  rotateCounterClockwise();
  delay(5000); // Adjust this duration based on your hardware
  stopRotation();
}
