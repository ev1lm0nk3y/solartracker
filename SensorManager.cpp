#include "SensorManager.h"

SensorManager::SensorManager(float maxTilt)
    : heading(0), pitch(0), tiltAngle(0), mpuTemp(0.0), 
      isFallen(false), maxTiltLimit(maxTilt), 
      mpuActive(false), compassActive(false) {}

bool SensorManager::beginMPU() {
    if (!mpu.begin()) {
        mpuActive = false;
        return false;
    }
    
    // Default config
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    mpuActive = true;
    return true;
}

void SensorManager::beginCompass() {
    compass.init();
    compassActive = true;
}

void SensorManager::update(bool debug) {
    // Update Compass
    if (compassActive) {
        compass.read();
        heading = compass.getAzimuth();
    }

    // Update MPU
    if (mpuActive) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        
        if (debug) {
            logMPUData(a, g, temp);
        }

        mpuTemp = temp.temperature;
        
        // Calculate Pitch (Angle around Y-axis)
        pitch = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;

        // Calculate Tilt (Safety check)
        float horizontalMag = sqrt(sq(a.acceleration.x) + sq(a.acceleration.y));
        tiltAngle = atan2(horizontalMag, abs(a.acceleration.z)) * 180.0 / PI;

        if (tiltAngle > maxTiltLimit) {
            isFallen = true;
        }
    }
}

void SensorManager::logMPUData(sensors_event_t a, sensors_event_t g, sensors_event_t temp) {
    Serial.print("Accel X: "); Serial.print(a.acceleration.x);
    Serial.print(", Y: "); Serial.print(a.acceleration.y);
    Serial.print(", Z: "); Serial.print(a.acceleration.z);
    Serial.print(" m/s^2 | Rotation X: "); Serial.print(g.gyro.x);
    Serial.print(", Y: "); Serial.print(g.gyro.y);
    Serial.print(", Z: "); Serial.print(g.gyro.z);
    Serial.print(" rad/s | Temp: "); Serial.print(temp.temperature);
    Serial.println(" degC");
}
