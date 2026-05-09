import re

with open('tracker.ino', 'r') as f:
    content = f.read()

updateLCD_code = """
void updateLCD() {
  if (!availability[LCD]) return;

  if (sensors.hasFallen()) {
    lcd.clear();
    vLcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! CRITICAL ERROR !!!");
    vLcd.setLine(0, "!!! CRITICAL ERROR !!!");
    lcd.setCursor(0, 1);
    lcd.print("  SYSTEM FALLEN  ");
    vLcd.setLine(1, "  SYSTEM FALLEN  ");
    lcd.setCursor(0, 2);
    lcd.print("Tilt Angle: ");
    lcd.print(sensors.getTiltAngle());
    vLcd.setLine(2, "Tilt Angle: " + String(sensors.getTiltAngle()));
    lcd.setCursor(0, 3);
    lcd.print("Please Reset System");
    vLcd.setLine(3, "Please Reset System");
    return;
  }

  if (isShutdown) return;  // Don't update LCD if sleeping

  lcd.clear();
  vLcd.clear();
  char buffer[21];

  // If in manual mode place a banner on all updates
  int startLine = 0;
  if (manualMode) {
    lcd.setCursor(0, 0);
    lcd.print("=== MANUAL  MODE ===");
    vLcd.setLine(0, "=== MANUAL  MODE ===");
    startLine = 1;
  }

  // Line 0: LDR Averages
  sprintf(buffer, "L:%-4d R:%-4d", ldrs.getLeftAverage(), ldrs.getRightAverage());
  lcd.setCursor(0, startLine);
  lcd.print(buffer);
  vLcd.setLine(startLine, String(buffer));

  // Line 1: Motor Status
  // If sensor error, show Manual Mode or Error
  if (sensorError) {
    lcd.setCursor(0, startLine + 1);
    lcd.print("SENSOR ERROR - MAN");
    vLcd.setLine(startLine + 1, "SENSOR ERROR - MAN");
  } else {
    sprintf(buffer, "P:%-4s R:%-4s", motors.getPitchStatus().c_str(), motors.getRotationStatus().c_str());
    lcd.setCursor(0, startLine + 1);
    lcd.print(buffer);
    vLcd.setLine(startLine + 1, String(buffer));
  }

  // Line 2: Heading & Pitch
  sprintf(buffer, "H:%-3d P:%-3d", sensors.getHeading(), sensors.getPitch());
  lcd.setCursor(0, startLine + 2);
  lcd.print(buffer);
  vLcd.setLine(startLine + 2, String(buffer));

  // Line 3: Temp & WiFi Status (Simplified)
  // Disable if in Manual Mode
  if (!manualMode) {
    char wifiStat = '\\1';                                  // Default to cross (Disconnected)
    if (wifi_server_status == LISTENING) wifiStat = '\\0';  // check (Listening)
    else if (status == WL_AP_LISTENING) wifiStat = '\\5';   // wifi symbol (AP Active)

    // Format: T:25.5*C WiFiSrv: V
    char stat_buffer[21];
    lcd.setCursor(0, 3);
    // Note: \\3 is degreeC symbol from register
    sprintf(stat_buffer, "T:%.1f\\3 WiFiSrv: %c", sensors.getTemp(), wifiStat);
    lcd.print(stat_buffer);
    
    // For virtual LCD, replace special chars with readable ones
    String vStatStr = (wifiStat == '\\0') ? "OK" : ((wifiStat == '\\1') ? "X" : "AP");
    sprintf(stat_buffer, "T:%.1fC WiFi:%s", sensors.getTemp(), vStatStr.c_str());
    vLcd.setLine(3, String(stat_buffer));
  }
}
"""

content = content.replace(
    '// --- Sensor & Display Functions ---\n',
    '// --- Sensor & Display Functions ---\n' + updateLCD_code + '\n'
)

with open('tracker.ino', 'w') as f:
    f.write(content)

print("Done")
