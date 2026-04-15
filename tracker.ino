/*
  Solar Tracker Code - Version 2.2

  This sketch implements a "follow the sun" routine for a dual-axis solar tracker.
  It uses four light sensors (LDRs) to determine the sun's position and controls
  two motors (for rotation and pitch) to keep a solar panel aimed at the sun.

  New features in V2.2:
  - Refactored LDR logic into LDRManager class
  - Updated for Arduino Uno R4 WiFi
  - I2C 20x4 LCD for diagnostics
  - GY-271 (QMC5883L) compass for heading
  - Startup routine to check components and home motors
  - Shutdown routine to flatten the panel in low light

  Hardware requirements:
  - Arduino Uno R4 WiFi
  - Arduino Motor Shield Rev3 (stacks on top)
  - 4 Light Dependent Resistors (LDRs)
  - Rotational motor (Motor A)
  - Pitch motor (Motor B)
  - 20x4 I2C LCD Display
  - GY-271 (QMC5883L) Compass
*/

// --- Libraries ---
#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>
#include "LDRManager.h"
#include "MotorController.h"
#include "SensorManager.h"

// --- WiFi Settings ---
char ssid[] = "SolarTrackerAP";
char pass[] = "12345678";
int status = WL_IDLE_STATUS;
int server_port = 80;
WiFiServer server(server_port);
IPAddress tracker_ip = IPAddress(192, 168, 4, 1);
enum ServerState {
  LISTENING,
  STARTING,
  STOPPED,
};
ServerState wifi_server_status = STOPPED;

// Custom symbols character
uint8_t check[8] = { 0x00, 0x01, 0x03, 0x16, 0x1c, 0x08, 0x00, 0x00 };
uint8_t cross[8] = { 0x00, 0x1b, 0x0e, 0x04, 0x0e, 0x1b, 0x00, 0x00 };
uint8_t degreeSymbol[8] = { 0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00 };
uint8_t degreeC[8] = { 0x18, 0x18, 0x03, 0x04, 0x04, 0x04, 0x03, 0x00 };
uint8_t uparrow[8] = { 0x04, 0x0e, 0x1f, 0x0e, 0x0e, 0x0e, 0x00, 0x00 };
uint8_t wifi[8] = { 0x00, 0x00, 0x00, 0x08, 0x16, 0x21, 0x00, 0x00 };

// #define MANUALMODE // Don't do auto solar tracking
#define DEBUG_MPU  // print out mpu data every collection
#define SCANI2CBUS // Scan I2C bus on startup to detect sensors

// --- Pin Definitions ---
const int LDR_TOP_LEFT_PIN = A0;
const int LDR_TOP_RIGHT_PIN = A1;
const int LDR_BOTTOM_LEFT_PIN = A2;
const int LDR_BOTTOM_RIGHT_PIN = A3;

// Motor Shield Rev3 Pins
const int MOTOR_A_DIR_PIN = 12;
const int MOTOR_A_PWM_PIN = 3;
const int MOTOR_A_BRAKE_PIN = 9;
const int MOTOR_B_DIR_PIN = 13;
const int MOTOR_B_PWM_PIN = 11;
const int MOTOR_B_BRAKE_PIN = 8;

// --- Constants ---
const int MOTOR_SPEED = 200;
const int TOLERANCE = 50;
const int SHUTDOWN_LIGHT_THRESHOLD = 150;
const long LCD_UPDATE_INTERVAL = 1000;
const int HEADING_LIMIT_MIN = 80;
const int HEADING_LIMIT_MAX = 280;
const int PITCH_LIMIT_MIN = 5;
const int PITCH_LIMIT_MAX = 75;
const int MPU_ADDR = 0x68;                   // I2C address of MPU6050
const float MAX_TILT_ANGLE = 45.0;
const long INITIALIZATION_DELAY_MS = 10000;

<<<<<<< Updated upstream
=======
// --- Classes ---
class VirtualLCD {
  public:
    String lines[4];
    VirtualLCD() { for(int i=0; i<4; i++) lines[i] = "                    "; }
    void clear() { for(int i=0; i<4; i++) lines[i] = "                    "; }
    void setLine(int row, String text) {
      if (row >= 0 && row < 4) {
        lines[row] = text;
        while(lines[row].length() < 20) lines[row] += " ";
        if(lines[row].length() > 20) lines[row] = lines[row].substring(0, 20);
      }
    }
};

>>>>>>> Stashed changes
// --- Objects ---
hd44780_I2Cexp lcd;
<<<<<<< Updated upstream
Adafruit_MPU6050 mpu;
QMC5883LCompass compass;
LDRManager ldrs(LDR_TOP_LEFT_PIN, LDR_TOP_RIGHT_PIN, LDR_BOTTOM_LEFT_PIN, LDR_BOTTOM_RIGHT_PIN);
=======
VirtualLCD vLcd;
LDRManager ldrs(LDR_TOP_LEFT_PIN, LDR_TOP_RIGHT_PIN, LDR_BOTTOM_LEFT_PIN, LDR_BOTTOM_RIGHT_PIN);
MotorController motors(MOTOR_A_DIR_PIN, MOTOR_A_PWM_PIN, MOTOR_A_BRAKE_PIN, 
                       MOTOR_B_DIR_PIN, MOTOR_B_PWM_PIN, MOTOR_B_BRAKE_PIN, MOTOR_SPEED);
SensorManager sensors(MAX_TILT_ANGLE);
>>>>>>> Stashed changes

// --- Global Variables ---
int deviceCount = 0;
bool isShutdown = false;
bool manualMode = false;
bool sensorError = false;
unsigned long lastManualCommandTime = 0;
const unsigned long MANUAL_TIMEOUT = 60000;

enum PERIPHERALS { LDR_TL, LDR_TR, LDR_BL, LDR_BR, MPU_SENSOR, COMPASS, MOTOR_A, MOTOR_B, LCD, WIFI, NUM_DEVICES };
bool availability[NUM_DEVICES];
unsigned long lastLcdUpdateTime = 0;

// Function prototypes
void writeLCD(char output[], uint8_t col = 0, uint8_t row = 0, bool clearFirst = true);
void handleClient(WiFiClient client);
void main_loop();
void scanI2C();
void startupRoutine();
void shutdownRoutine();
void updateLCD();
void flattenPitchMotor();
void homeRotationMotor();

// setup and loops
void setup() {
#ifdef MANUALMODE
  manualMode = true;
#endif

  for (int i = 0; i < NUM_DEVICES; i++) {
    availability[i] = false;
  }

  Serial.begin(9600);
  Serial.println("Initializing...");

  // Initialize Motors
  motors.begin();
  motors.setLimits(PITCH_LIMIT_MIN, PITCH_LIMIT_MAX, HEADING_LIMIT_MIN, HEADING_LIMIT_MAX);

  // Initialize WiFi AP
  Serial.print("- WiFi: ");
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("AP Failed\n");
    wifi_server_status = STOPPED;
  } else {
    Serial.print(" COMPLETE\n");
    Serial.print("- Webserver: ");
    server.begin();
    tracker_ip = WiFi.softAPIP();
    Serial.println("http://" + tracker_ip.toString() + "\n");
    availability[WIFI] = true;
    wifi_server_status = LISTENING;
  }

  startupRoutine();
}

void manual_loop() {}

void loop() {
  if (manualMode) {
    manual_loop();
  }

  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  main_loop();
}

void handleClient(WiFiClient client) {
  String currentLine = "";
  String requestLine = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (requestLine.length() < 100) requestLine += c;
      if (c == '\n') {
        if (currentLine.length() == 0) {
          // End of HTTP headers, send response

          // Basic API routing based on requestLine
          if (requestLine.indexOf("GET /data") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();

            StaticJsonDocument<256> doc;
            
            JsonArray ldrArray = doc.createNestedArray("ldr");
            ldrArray.add(ldrs.getTopLeft());
            ldrArray.add(ldrs.getTopRight());
            ldrArray.add(ldrs.getBottomLeft());
            ldrArray.add(ldrs.getBottomRight());
            
            doc["heading"] = sensors.getHeading();
            doc["pitch"] = sensors.getPitch();
            doc["manual"] = manualMode;
<<<<<<< Updated upstream
            doc["fallen"] = isFallen;
            doc["tilt"] = tiltAngle;
=======
            doc["fallen"] = sensors.hasFallen();
            doc["tilt"] = sensors.getTiltAngle();
            doc["pitchStatus"] = motors.getPitchStatus();
            doc["rotationStatus"] = motors.getRotationStatus();

            JsonArray lcdLines = doc.createNestedArray("lcd");
            for(int i=0; i<4; i++) lcdLines.add(vLcd.lines[i]);
>>>>>>> Stashed changes

            serializeJson(doc, client);
          } else if (requestLine.indexOf("GET /cmd") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();

            if (sensors.hasFallen()) {
              client.print("ERROR: SYSTEM FALLEN");
            } else {
              manualMode = true;
              lastManualCommandTime = millis();

              if (requestLine.indexOf("action=left") >= 0) motors.rotateCCW(sensors.getHeading());
              else if (requestLine.indexOf("action=right") >= 0) motors.rotateCW(sensors.getHeading());
              else if (requestLine.indexOf("action=up") >= 0) motors.pitchUp(sensors.getPitch());
              else if (requestLine.indexOf("action=down") >= 0) motors.pitchDown(sensors.getPitch());
              else if (requestLine.indexOf("action=stop") >= 0) {
                motors.stopAll();
              } else if (requestLine.indexOf("action=auto") >= 0) {
                manualMode = false;
                motors.stopAll();
              }
              client.print("OK");
            }
          } else {
            // Serve Interface
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();

            client.println("<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head>");
            client.println("<style>button{width:100px;height:50px;font-size:20px;margin:5px;} .pad{display:grid;grid-template-columns:1fr 1fr 1fr;width:320px;margin:auto;}</style>");
            client.println("<body><center><h1>Solar Tracker</h1>");
            client.println("<div id='data'>Loading...</div>");
            client.println("<div class='pad'>");
            client.println("<div></div><button onmousedown='c(\"up\")' onmouseup='c(\"stop\")'>UP</button><div></div>");
            client.println("<button onmousedown='c(\"left\")' onmouseup='c(\"stop\")'>LEFT</button><button onclick='c(\"auto\")'>AUTO</button><button onmousedown='c(\"right\")' onmouseup='c(\"stop\")'>RIGHT</button>");
            client.println("<div></div><button onmousedown='c(\"down\")' onmouseup='c(\"stop\")'>DOWN</button><div></div>");
            client.println("</div>");
            client.println("<script>function c(a){fetch('/cmd?action='+a);}");
            client.println("setInterval(()=>{fetch('/data').then(r=>r.json()).then(d=>{ ");
            client.println("document.getElementById('data').innerHTML='Heading: '+d.heading+' Pitch: '+d.pitch+'<br>LDRs: '+d.ldr.join(',');");
            client.println("if(d.fallen){document.getElementById('data').innerHTML+='<br><h2 style=\"color:red\">SYSTEM FALLEN! TILT: '+d.tilt+'</h2>';}");
            client.println("});}, 1000);</script>");
            client.println("</center></body></html>");
          }
          break;
        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }
  client.stop();
}

void main_loop() {
  // Update sensors
  ldrs.update();
  if (!sensorError) {
    sensors.update(false); // pass true for serial debug
  }

  if (sensors.hasFallen()) {
    motors.stopAll();
    return;
  }

  int avgLight = ldrs.getTotalAverage();

  // Shutdown logic
  if (avgLight < SHUTDOWN_LIGHT_THRESHOLD && !isShutdown && millis() > INITIALIZATION_DELAY_MS) {
    shutdownRoutine();
  } else if (avgLight >= SHUTDOWN_LIGHT_THRESHOLD) {
    if (isShutdown) {
      isShutdown = false;
      writeLCD("Waking up...");
      delay(2000);
    }

    // Tracking Logic
    int vDiff = ldrs.getVerticalDiff();
    int hDiff = ldrs.getHorizontalDiff();

    if (abs(vDiff) > TOLERANCE) {
      if (vDiff > 0) motors.pitchDown(sensors.getPitch());
      else motors.pitchUp(sensors.getPitch());
    } else {
      motors.stopPitch();
    }

    if (abs(hDiff) > TOLERANCE) {
      if (hDiff > 0) motors.rotateCCW(sensors.getHeading());
      else motors.rotateCW(sensors.getHeading());
    } else {
      motors.stopRotation();
    }
  }

  // UI Update
  if (millis() - lastLcdUpdateTime > LCD_UPDATE_INTERVAL) {
    updateLCD();
    lastLcdUpdateTime = millis();
  }

  delay(100);
}

// --- Primary Routines ---


void writeLCD(char output[], uint8_t col, uint8_t row, bool clearFirst) {
  if (!availability[LCD]) { return; }
  if (clearFirst) {
    lcd.clear();
  }
  lcd.setCursor(col, row);
  lcd.print(output);
}

void scanI2C() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus...");
  writeLCD("Scanning I2C...");

  // Reset critical flags (LCD availability handled by begin() return)
  availability[MPU_SENSOR] = false;
  availability[COMPASS] = false;

  String deviceList = "";

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;

      if (address == MPU_ADDR) availability[MPU_SENSOR] = true;
      if (address == 0x0D) availability[COMPASS] = true;

      if (nDevices <= 4) {
        if (deviceList.length() > 0) deviceList += ",";
        deviceList += String(address, HEX);
      }
    }
  }

  deviceCount = nDevices;
  lcd.lineWrap();
  char lcdbuf[21];
  sprintf(lcdbuf, "Found %d devices", nDevices);
  writeLCD(lcdbuf, 0, 1, false);
  sprintf(lcdbuf, "Devs: %s", deviceList.c_str());
  writeLCD(lcdbuf, 0, 2, false);
  delay(500);
}

void startupRoutine() {
  Wire.begin();

  Serial.write("LCD Starting... ");
  if (lcd.begin(20, 4) != 0) {
    Serial.write("Failed\n");
  } else {
    availability[LCD] = true;
    lcd.backlight();
    lcd.home();
    lcd.print("Starting...");
    lcd.createChar(0, check);
    lcd.createChar(1, cross);
    lcd.createChar(2, degreeSymbol);
    lcd.createChar(3, degreeC);
    lcd.createChar(4, uparrow);
    lcd.createChar(5, wifi);
  }

#ifdef SCANI2CBUS
  scanI2C();
#endif

  // MPU Setup
  writeLCD("MPU6050... ", 0, 1, false);
  if (!sensors.beginMPU()) {
    Serial.println("MPU Init Failed!");
    sensorError = true;
    writeLCD("FAILED!", 11, 1, false);
  } else {
    Serial.println("MPU6050 Active");
    writeLCD("READY", 11, 1, false);
  }

  // Compass Setup
  sensors.beginCompass();
  Serial.println("Compass Active");

  if (sensorError) {
    Serial.println("CRITICAL: Sensors missing. Entering MANUAL MODE ONLY.");
    manualMode = true;
    writeLCD("+++ SENSOR ERROR! +++");
    writeLCD("Manual Mode Only", 0, 1, false);
    delay(2000);
    return;
  }

  writeLCD("Homing motors... ");
  flattenPitchMotor();
  homeRotationMotor();

  writeLCD("Startup Complete.");
  delay(1000);
}

void shutdownRoutine() {
  isShutdown = true;
  Serial.println("Low light detected. Shutting down.");
  writeLCD("Low light detected.");
  writeLCD("Shutting down...", 0, 1, false);

  motors.stopAll();

  if (!sensorError) {
    flattenPitchMotor();
  }
  writeLCD("System asleep.");
<<<<<<< Updated upstream
  Serial.println("System asleep.");
}

// --- Sensor & Display Functions ---

void readSensors() {
  // Now handled by LDRManager class call in main_loop
  ldrs.update();
}

void readCompass() {
  compass.read();
  heading = compass.getAzimuth();
}

void logMPUData(sensors_event_t a, sensors_event_t g, sensors_event_t temp) {
  Serial.print("Accel X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.print(" m/s^2");

  Serial.print(" | Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.print(" rad/s");

  Serial.print(" | Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");
}

void readPitch() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
#ifdef DEBUG_MPU
  logMPUData(a, g, temp);
#endif

  // Store temp for LCD
  mpuTemp = temp.temperature;

  // Log data
  // logMPUData(a, g, temp); // Uncomment to log every cycle (spammy)

  // Calculate Pitch (Angle around Y-axis)
  // Using accelerometer data (gravity vector)
  // pitch = atan2(accelerationY, accelerationZ) * 180/PI
  pitch = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;

  // --- Safety Check: Tilt Detection ---
  // Calculate angle of deviation from vertical (Z-axis)
  float horizontalMag = sqrt(sq(a.acceleration.x) + sq(a.acceleration.y));
  // Result in degrees. abs(a.acceleration.z) ensures we get the angle from the vertical axis.
  tiltAngle = atan2(horizontalMag, abs(a.acceleration.z)) * 180.0 / PI;

  if (tiltAngle > MAX_TILT_ANGLE) {
    if (!isFallen) {
      Serial.print("CRITICAL: Tracker has fallen! Tilt: ");
      Serial.println(tiltAngle);
      isFallen = true;
    }
  }
}

void updateLCD() {
  if (!availability[LCD]) return;

  if (isFallen) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! CRITICAL ERROR !!!");
    lcd.setCursor(0, 1);
    lcd.print("  SYSTEM FALLEN  ");
    lcd.setCursor(0, 2);
    lcd.print("Tilt Angle: ");
    lcd.print(tiltAngle);
    lcd.setCursor(0, 3);
    lcd.print("Please Reset System");
    return;
  }

  if (isShutdown) return;  // Don't update LCD if sleeping

  lcd.clear();
  char buffer[21];

  // If in manual mode place a banner on all updates
  int startLine = 0;
  if (manualMode) {
    lcd.setCursor(0, 0);
    lcd.print("==== MANUAL MODE ====");
    startLine = 1;
  }

  // Line 0: LDR Averages
  sprintf(buffer, "L:%-4d R:%-4d", ldrs.getLeftAverage(), ldrs.getRightAverage());
  lcd.setCursor(0, startLine);
  lcd.print(buffer);

  // Line 1: Motor Status
  // If sensor error, show Manual Mode or Error
  if (sensorError) {
    lcd.setCursor(0, startLine + 1);
    lcd.print("SENSOR ERROR - MAN");
  } else {
    sprintf(buffer, "P:%-4s R:%-4s", pitchStatus.c_str(), rotationStatus.c_str());
    lcd.setCursor(0, startLine + 1);
    lcd.print(buffer);
  }

  // Line 2: Heading & Pitch
  sprintf(buffer, "H:%-3d P:%-3d", heading, pitch);
  lcd.setCursor(0, startLine + 2);
  lcd.print(buffer);

  // Line 3: Temp & WiFi Status (Simplified)
  // Disable if in Manual Mode
  if (!manualMode) {
    char wifiStat = '\1';                                  // Default to cross (Disconnected)
    if (wifi_server_status == LISTENING) wifiStat = '\0';  // check (Listening)
    else if (status == WL_AP_LISTENING) wifiStat = '\5';   // wifi symbol (AP Active)

    // Format: T:25.5*C WiFiSrv: V
    char stat_buffer[21];
    lcd.setCursor(0, 3);
    // Note: \3 is degreeC symbol from register
    sprintf(stat_buffer, "T:%.1f\3 WiFiSrv: %c", mpuTemp, wifiStat);
    lcd.print(stat_buffer);
  }
}


// --- Motor Control Functions ---

void pitchUp() {
  if (pitch >= PITCH_LIMIT_MAX) {
    stopPitch();
    pitchStatus = "MAX";
    return;
  }
  digitalWrite(MOTOR_B_DIR_PIN, HIGH);
  analogWrite(MOTOR_B_PWM_PIN, MOTOR_SPEED);
  pitchStatus = "UP";
}

void pitchDown() {
  if (pitch <= PITCH_LIMIT_MIN) {
    stopPitch();
    pitchStatus = "MIN";
    return;
  }
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

void homePitchMotor() {
  // Not used if we flatten instead
  flattenPitchMotor();
=======
>>>>>>> Stashed changes
}

void flattenPitchMotor() {
  if (sensorError) return;
  Serial.println("Flattening Pitch...");
  unsigned long startTime = millis();

  while (sensors.getPitch() > PITCH_LIMIT_MIN && millis() - startTime < 15000) {
    sensors.update();
    motors.pitchDown(sensors.getPitch());
    delay(50);
  }
  motors.stopPitch();
}

void homeRotationMotor() {
  Serial.println("Homing Rotation (placeholder)...");
  motors.rotateCCW(sensors.getHeading());
  delay(5000);
  motors.stopRotation();
}