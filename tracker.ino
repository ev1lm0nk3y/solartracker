/*
  Solar Tracker Code - Version 2.1

  This sketch implements a "follow the sun" routine for a dual-axis solar tracker.
  It uses four light sensors (LDRs) to determine the sun's position and controls
  two motors (for rotation and pitch) to keep a solar panel aimed at the sun.

  New features in V2.1:
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
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <QMC5883LCompass.h>
#include <WiFiS3.h>

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
uint8_t check[8] = {0x00,0x01,0x03,0x16,0x1c,0x08,0x00,0x00};
uint8_t cross[8] = {0x00,0x1b,0x0e,0x04,0x0e,0x1b,0x00,0x00};
uint8_t degreeSymbol[8]= {0x06,0x09,0x09,0x06,0x00,0x00,0x00,0x00};
uint8_t degreeC[8]     = {0x18,0x18,0x03,0x04,0x04,0x04,0x03,0x00};
uint8_t uparrow[8]  = {0x04,0x0e,0x1f,0x0e,0x0e,0x0e,0x00,0x00};
uint8_t wifi[8] = {0x00,0x00,0x00,0x08,0x16,0x21,0x00,0x00};

#define MANUALMODE

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
const long LCD_UPDATE_INTERVAL = 1000; // Update LCD every 60000ms
const int HEADING_LIMIT_MIN = 80;   // Minimum angle (e.g., East limit)
const int HEADING_LIMIT_MAX = 280;  // Maximum angle (e.g., West limit)
const int PITCH_LIMIT_MIN = 5;      // Minimum angle (degrees) - Flat
const int PITCH_LIMIT_MAX = 75;     // Maximum angle (degrees) - Upright
const int MPU_ADDR = 0x68;          // I2C address of MPU6050
const float MAX_TILT_ANGLE = 45.0;  // Maximum allowed tilt angle from vertical

// --- Objects ---

// LCD: Auto-detect address
hd44780_I2Cexp lcd;
Adafruit_MPU6050 mpu;
QMC5883LCompass compass;

// --- Global Variables ---

// Sensor values
int ldrTopLeft, ldrTopRight, ldrBottomLeft, ldrBottomRight;
int avgTop, avgBottom, avgLeft, avgRight, avgLight;
int verticalDiff, horizontalDiff;
int deviceCount = 0; // Number of I2C devices found

// State variables
bool isShutdown = false;
bool isFallen = false;
bool manualMode = false;
bool sensorError = false; // New flag for hardware failures
unsigned long lastManualCommandTime = 0;
const unsigned long MANUAL_TIMEOUT = 60000; // 1 minute timeout
String pitchStatus = "STOP";
String rotationStatus = "STOP";
int heading = 0;
int pitch = 0;
int tiltAngle = 0; // Store overall tilt for display/logging
float mpuTemp = 0; // Global to store temp for display

// Peripherals: LCD, motor and sensor availability
// 0-3: LDR Top Left, Top Right, Bottom Left, Bottom Right
// 4: MPU6050
// 5: Compass
// 6: Motor A
// 7: Motor B
// 8: LCD
// 9: WiFi AP
enum PERIPHERALS {
  LDR_TL,
  LDR_TR,
  LDR_BL,
  LDR_BR,
  MPU_SENSOR,
  COMPASS,
  MOTOR_A,
  MOTOR_B,
  LCD,
  WIFI,
  NUM_DEVICES
};
bool availability[NUM_DEVICES];

// Timing variables
unsigned long lastLcdUpdateTime = 0;

// Function prototype

// Generic function to print lines onto the LCD
void writeLCD(char output[], uint8_t col = 0, uint8_t row = 0, bool clearFirst = true);


void setup() {
#ifdef MANUALMODE
  manualMode = true;
#endif

  for (int i = 0; i < NUM_DEVICES; i++) {
    availability[i] = false;
  }

  Serial.begin(9600);

  Serial.print("Initializing...");

  // Initialize Motor Pins
  pinMode(MOTOR_A_DIR_PIN, OUTPUT);
  pinMode(MOTOR_A_PWM_PIN, OUTPUT);
  pinMode(MOTOR_A_BRAKE_PIN, OUTPUT);
  digitalWrite(MOTOR_A_BRAKE_PIN, LOW); // Disable brake

  pinMode(MOTOR_B_PWM_PIN, OUTPUT);
  pinMode(MOTOR_B_BRAKE_PIN, OUTPUT);
  digitalWrite(MOTOR_B_BRAKE_PIN, LOW); // Disable brake

  // Initialize WiFi AP
  Serial.print("- WiFi: ");
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("AP Failed\n");
    // Don't halt, just continue without WiFi
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

void loop() {
  // --- WiFi Client Handling ---
  WiFiClient client = server.available();
  if (client) {
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

              client.print("{");
              client.print("\"ldr\":[");
              client.print(ldrTopLeft); client.print(",");
              client.print(ldrTopRight); client.print(",");
              client.print(ldrBottomLeft); client.print(",");
              client.print(ldrBottomRight);
              client.print("],");
              client.print("\"heading\":"); client.print(heading); client.print(",");
              client.print("\"pitch\":"); client.print(pitch); client.print(",");
              client.print("\"manual\":"); client.print(manualMode ? "true" : "false"); client.print(",");
              client.print("\"fallen\":"); client.print(isFallen ? "true" : "false"); client.print(",");
              client.print("\"tilt\":"); client.print(tiltAngle);
              client.print("}");
            }
            else if (requestLine.indexOf("GET /cmd") >= 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/plain");
              client.println("Connection: close");
              client.println();

              if (isFallen) {
                 client.print("ERROR: SYSTEM FALLEN");
              } else {
                  manualMode = true;
                  lastManualCommandTime = millis();

                  if (requestLine.indexOf("action=left") >= 0) rotateCounterClockwise();
                  else if (requestLine.indexOf("action=right") >= 0) rotateClockwise();
                  else if (requestLine.indexOf("action=up") >= 0) pitchUp();
                  else if (requestLine.indexOf("action=down") >= 0) pitchDown();
                  else if (requestLine.indexOf("action=stop") >= 0) {
                    stopRotation();
                    stopPitch();
                  }
                  else if (requestLine.indexOf("action=auto") >= 0) {
                    manualMode = false;
                    stopRotation();
                    stopPitch();
                  }
                  client.print("OK");
              }
            }
            else {
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

  // --- Main Logic ---

  readSensors();
  if (!sensorError) {
    readCompass(); // Update heading for limit checks
    readPitch();   // Update pitch for limit checks and check safety
  }

  if (isFallen) {
    stopRotation();
    stopPitch();
    // In fallen state, we do not proceed with tracking
    // We still allow LCD updates to show the error
  }
  else {
    avgLight = (ldrTopLeft + ldrTopRight + ldrBottomLeft + ldrBottomRight) / 4;

    // Auto-revert from manual mode
    if (manualMode) {
      writeLCD("==== MANUAL MODE ====");
      Serial.println("Manual mode detected, tracking disabled.");
    }
    else if (avgLight < SHUTDOWN_LIGHT_THRESHOLD && !isShutdown) {
      shutdownRoutine();
    } else if (avgLight >= SHUTDOWN_LIGHT_THRESHOLD) {
      // ... (Existing Tracking Logic) ...
      if(isShutdown) {
        // Waking up from shutdown
        isShutdown = false;
        writeLCD("Waking up...");
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
  }

  // Periodically update diagnostics
  if (millis() - lastLcdUpdateTime > LCD_UPDATE_INTERVAL) {
    updateLCD();

    // Log MPU Data
    if (!sensorError) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      logMPUData(a, g, temp);
    }

    lastLcdUpdateTime = millis();
  }

  delay(100); // Main loop delay
}

// --- Primary Routines ---


void writeLCD(char output[], uint8_t col, uint8_t row, bool clearFirst) {
  if (!availability[LCD]) {return;}
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

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
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

  // Initialize LCD (HD44780)
  // begin returns 0 on success
  Serial.write("LCD Starting... ");
  if (lcd.begin(20, 4) != 0) {
    Serial.write("Failed\n");
  } else {
    availability[LCD] = true;
    lcd.backlight();
    lcd.home();
    lcd.print("Starting...");
    Serial.write("COMPLETE\n");

    // Putting the LCD custom symbols into the register
    lcd.createChar(0, check);
    lcd.createChar(1, cross);
    lcd.createChar(2, degreeSymbol);
    lcd.createChar(3, degreeC);
    lcd.createChar(4, uparrow);
    lcd.createChar(5, wifi);
  }

#ifdef SCANI2CBUS
  scanI2C();
  availability[MPU_SENSOR] = true;
  availability[COMPASS] = true;
#endif

  // Check MPU
  if (availability[MPU_SENSOR]) {
      writeLCD("MPU6050... ", 0, 1, false);
      if (!mpu.begin()) {
        Serial.println("MPU Init Failed!");
        sensorError = true;
        writeLCD("FAILED!", 11, 1, false);
      } else {
        Serial.println("MPU6050 Active");
        // >>> PASTE MPU6050 CALIBRATION OFFSETS HERE (from calibrate.ino) <<<
        // Example:
        // mpu.setAccelerometerOffset(-200, 100, 1000);
        // mpu.setGyroOffset(5, -10, 2);
        mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        writeLCD("READY", 11, 1, false);
      }
  } else {
      Serial.println("MPU6050 Missing!");
      sensorError = true;
  }

  // Check Compass
  if (availability[COMPASS]) {
      compass.init();
      // >>> PASTE QMC5883L COMPASS CALIBRATION HERE (from QMC5883LCompass example sketch) <<<
      // Example:
      // compass.setCalibration(-1537, 1266, -1961, 958, -1342, 1492);
      Serial.println("Compass Active");
  } else {
      Serial.println("Compass Missing!");
      // sensorError = true; // Optional: Treat compass as critical? Yes, for heading limits.
      sensorError = false;
  }

  if (sensorError) {
      Serial.println("CRITICAL: Sensors missing. Entering MANUAL MODE ONLY.");
      manualMode = true;
      writeLCD("+++ SENSOR ERROR! +++");
      writeLCD("Manual Mode Only", 0, 1, false);
      delay(2000);
      return; // Skip homing
  }

  writeLCD("Homing motors... ");
  Serial.println("Homing motors...");

  // --- Home Motors ---
  flattenPitchMotor(); // Use flatten as homing for pitch
  homeRotationMotor();

  writeLCD("Startup Complete.");
  Serial.println("Startup Complete.");
  delay(1000);
}

void shutdownRoutine() {
  isShutdown = true;
  Serial.println("Low light detected. Shutting down.");
  if (availability[8]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Low light detected.");
    lcd.setCursor(0, 1);
    lcd.print("Shutting down...");
  }

  stopRotation();

  // --- Flatten Panel ---
  if (!sensorError) {
    flattenPitchMotor();
  }

  writeLCD("System asleep.");
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

void logMPUData(sensors_event_t a, sensors_event_t g, sensors_event_t temp) {
  Serial.print("Accel X: "); Serial.print(a.acceleration.x);
  Serial.print(", Y: "); Serial.print(a.acceleration.y);
  Serial.print(", Z: "); Serial.print(a.acceleration.z);
  Serial.print(" m/s^2");

  Serial.print(" | Rotation X: "); Serial.print(g.gyro.x);
  Serial.print(", Y: "); Serial.print(g.gyro.y);
  Serial.print(", Z: "); Serial.print(g.gyro.z);
  Serial.print(" rad/s");

  Serial.print(" | Temperature: "); Serial.print(temp.temperature);
  Serial.println(" degC");
}

void readPitch() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

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
    lcd.setCursor(0,0);
    lcd.print("!!! CRITICAL ERROR !!!");
    lcd.setCursor(0,1);
    lcd.print("  SYSTEM FALLEN  ");
    lcd.setCursor(0,2);
    lcd.print("Tilt Angle: ");
    lcd.print(tiltAngle);
    lcd.setCursor(0,3);
    lcd.print("Please Reset System");
    return;
  }

  if(isShutdown) return; // Don't update LCD if sleeping

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
  sprintf(buffer, "L:%-4d R:%-4d", avgLeft, avgRight);
  lcd.setCursor(0,startLine);
  lcd.print(buffer);

  // Line 1: Motor Status
  // If sensor error, show Manual Mode or Error
  if (sensorError) {
      lcd.setCursor(0,startLine+1);
      lcd.print("SENSOR ERROR - MAN");
  } else {
      sprintf(buffer, "P:%-4s R:%-4s", pitchStatus.c_str(), rotationStatus.c_str());
      lcd.setCursor(0,startLine+1);
      lcd.print(buffer);
  }

  // Line 2: Heading & Pitch
  sprintf(buffer, "H:%-3d P:%-3d", heading, pitch);
  lcd.setCursor(0,startLine+2);
  lcd.print(buffer);

  // Line 3: Temp & WiFi Status (Simplified)
  // Disable if in Manual Mode
  if (!manualMode) {
    char wifiStat = '\1'; // Default to cross (Disconnected)
    if (wifi_server_status == LISTENING) wifiStat = '\0'; // check (Listening)
    else if (status == WL_AP_LISTENING) wifiStat = '\5'; // wifi symbol (AP Active)

    // Format: T:25.5*C WiFiSrv: V
    char stat_buffer[21];
    lcd.setCursor(0,3);
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

void homePitchMotor(){
  // Not used if we flatten instead
  flattenPitchMotor();
}

void flattenPitchMotor(){
  if (sensorError) return;
  Serial.println("Flattening Pitch to Limit...");
  unsigned long startTime = millis();

  // Run until pitch hits minimum or timeout (safety)
  while (pitch > PITCH_LIMIT_MIN && millis() - startTime < 15000) {
    readPitch(); // Keep updating pitch!
    pitchDown();
    delay(50);
  }
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
