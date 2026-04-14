/*
  Linear Actuator (Pitch Motor) Test Program
  Validates the actuator goes in and out while connected to the frame.
  Takes manual input via Serial Monitor.
  
  Motor B on Arduino Motor Shield Rev3:
  - Direction: D13
  - PWM (Speed): D11
  - Brake: D8
*/

const int DIR_PIN = 13;
const int PWM_PIN = 11;
const int BRAKE_PIN = 8;

void setup() {
  Serial.begin(115200);
  
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(BRAKE_PIN, OUTPUT);
  
  // Initial state: stopped
  digitalWrite(BRAKE_PIN, LOW);
  analogWrite(PWM_PIN, 0);
  
  Serial.println("========================================");
  Serial.println(" Linear Actuator (Motor B) Test Tool");
  Serial.println("========================================");
  Serial.println("Usage: Type three numbers separated by spaces and press Enter.");
  Serial.println("Format: <direction> <speed> <duration_in_seconds>");
  Serial.println("  - direction: 1 (Extend/Forward) or 0 (Retract/Reverse)");
  Serial.println("  - speed: 0 to 255 (0 = stop, 255 = max speed)");
  Serial.println("  - duration: Time in seconds to run (e.g., 2.5)");
  Serial.println("Example: 1 255 3.0  (Extend at full speed for 3 seconds)");
  Serial.println("========================================");
  Serial.println("Ready for input:");
}

void loop() {
  if (Serial.available() > 0) {
    int dir = Serial.parseInt();
    int speed = Serial.parseInt();
    float duration = Serial.parseFloat();
    
    // Clear the rest of the serial buffer (like newline characters)
    while (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n') break;
    }
    
    // Validate inputs
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;
    if (dir != 0 && dir != 1) dir = 1; // Default to 1 if invalid
    
    if (duration > 0) {
      Serial.print("\nExecuting Command -> Direction: ");
      Serial.print(dir == 1 ? "EXTEND (1)" : "RETRACT (0)");
      Serial.print(" | Speed: ");
      Serial.print(speed);
      Serial.print(" | Duration: ");
      Serial.print(duration);
      Serial.println(" seconds");
      
      // Release brake, set direction, and apply PWM
      digitalWrite(BRAKE_PIN, LOW);
      digitalWrite(DIR_PIN, dir ? HIGH : LOW);
      analogWrite(PWM_PIN, speed);
      
      // Wait for the specified duration
      unsigned long delayMs = duration * 1000.0;
      delay(delayMs);
      
      // Stop motor and apply brake
      analogWrite(PWM_PIN, 0);
      digitalWrite(BRAKE_PIN, HIGH);
      
      Serial.println("Test complete. Motor stopped.");
      Serial.println("Ready for next input:");
    }
  }
}
