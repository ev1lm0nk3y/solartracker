#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <QMC5883LCompass.h>

Adafruit_MPU6050 mpu;
QMC5883LCompass compass;

void setup() {
  Serial.begin(115200);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 sensor!");
  } else {
    calibrateMPU();
  }

  // Call compass calibration instructions
  calibrateCompass();
}

void calibrateMPU() {
  // Only calibrate if specifically requested or for debugging offsets.
  // Ideally, run this ONCE with the device FLAT, copy the printed values,
  // and put them in startupRoutine(), then comment out the call to this function.

  Serial.println(F("---------------------------------------"));
  Serial.println(F("CALIBRATING MPU6050... KEEP FLAT & STILL"));
  Serial.println(F("---------------------------------------"));

  long sumAx = 0, sumAy = 0, sumAz = 0;
  long sumGx = 0, sumGy = 0, sumGz = 0;
  int numReadings = 500; // Changed from const to allow reassignment

  Serial.print("Number of readings [" + String(numReadings) + "]: "); // Convert numReadings to String
  numReadings = Serial.readString().toInt();
  if (numReadings == 0) numReadings = 500; // Default if no input or invalid input

  // We need RAW values for offset calculation, not the m/s^2 floats.
  // Adafruit library hides raw values, so we read directly from registers for calibration.
  // This assumes standard default ranges.
  for (int i = 0; i < numReadings; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Convert back to approximate raw LSB for estimation
    // (default 8G range = 4096 LSB/g, 500deg/s = 65.5 LSB/deg/s)
    // This provides a "good enough" starting point for manual tuning if needed.
    sumAx += (int)(a.acceleration.x * 4096.0 / 9.81);
    sumAy += (int)(a.acceleration.y * 4096.0 / 9.81);
    sumAz += (int)(a.acceleration.z * 4096.0 / 9.81);

    sumGx += (int)(g.gyro.x * 65.5);
    sumGy += (int)(g.gyro.y * 65.5);
    sumGz += (int)(g.gyro.z * 65.5);
    delay(3);
  }

  // Calculate Averages
  int avgAx = sumAx / numReadings;
  int avgAy = sumAy / numReadings;
  int avgAz = sumAz / numReadings; // Should be 4096 (1g) if flat
  int avgGx = sumGx / numReadings;
  int avgGy = sumGy / numReadings;
  int avgGz = sumGz / numReadings;

  // Calculate required offsets (Target is 0 for all except Z-accel which is 1G)
  // Note: These are rough estimates. For perfect 0, you might need to tune.

  Serial.println(F("\n>>> COPY THE LINES BELOW INTO startupRoutine() <<<"));
  Serial.println(F("    // -- MPU6050 Calibration Offsets --"));

  Serial.print(F("    mpu.setAccelerometerOffset("));
  Serial.print(-avgAx); Serial.print(F(", "));
  Serial.print(-avgAy); Serial.print(F(", "));
  Serial.print(4096 - avgAz); Serial.println(F(");")); // Compensate for 1g gravity

  Serial.print(F("    mpu.setGyroOffset("));
  Serial.print(-avgGx); Serial.print(F(", "));
  Serial.print(-avgGy); Serial.print(F(", "));
  Serial.print(-avgGz); Serial.println(F(");"));

  Serial.println(F("---------------------------------------"));
}

void calibrateCompass() {
    compass.init();
    Serial.println("This will provide calibration settings for your QMC5883L chip. When prompted, move the magnetometer in all directions until the calibration is complete.");
    Serial.println("Calibration will begin in:");
    for (i = 5; i > 0; i--) {
        Serial.println(" " + i + "  seconds");
        delay(1000);
    }

    Serial.println("CALIBRATING. Keep moving your sensor...");
    compass.calibrate();

    Serial.println("DONE. Copy the lines below and paste it into your projects sketch.);");
    Serial.println();
    Serial.print("compass.setCalibrationOffsets(");
    Serial.print(compass.getCalibrationOffset(0));
    Serial.print(", ");
    Serial.print(compass.getCalibrationOffset(1));
    Serial.print(", ");
    Serial.print(compass.getCalibrationOffset(2));
    Serial.println(");");
    Serial.print("compass.setCalibrationScales(");
    Serial.print(compass.getCalibrationScale(0));
    Serial.print(", ");
    Serial.print(compass.getCalibrationScale(1));
    Serial.print(", ");
    Serial.print(compass.getCalibrationScale(2));
    Serial.println(");");
  }

  void loop() {
    delay(1000);
  }
