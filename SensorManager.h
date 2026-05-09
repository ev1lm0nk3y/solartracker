#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <QMC5883LCompass.h>

class SensorManager {
  private:
    Adafruit_MPU6050 mpu;
    QMC5883LCompass compass;
    
    int heading;
    int pitch;
    int tiltAngle;
    float mpuTemp;
    bool isFallen;
    float maxTiltLimit;
    bool mpuActive;
    bool compassActive;

    void logMPUData(sensors_event_t a, sensors_event_t g, sensors_event_t temp);

  public:
    SensorManager(float maxTilt);
    
    bool beginMPU();
    void beginCompass();
    void update(bool debug = false);

    // Getters
    int getHeading() const { return heading; }
    int getPitch() const { return pitch; }
    int getTiltAngle() const { return tiltAngle; }
    float getTemp() const { return mpuTemp; }
    bool hasFallen() const { return isFallen; }
    bool isMPUActive() const { return mpuActive; }
    bool isCompassActive() const { return compassActive; }

    void resetFallen() { isFallen = false; }
};

#endif
