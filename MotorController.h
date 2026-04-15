#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

class MotorController {
  private:
    uint8_t pinDirA, pinPwmA, pinBrakeA;
    uint8_t pinDirB, pinPwmB, pinBrakeB;
    int motorSpeed;
    int pitchMin, pitchMax;
    int headingMin, headingMax;
    String pitchStatus;
    String rotationStatus;

  public:
    MotorController(uint8_t dirA, uint8_t pwmA, uint8_t brakeA,
                    uint8_t dirB, uint8_t pwmB, uint8_t brakeB, 
                    int speed);
    
    void setLimits(int pMin, int pMax, int hMin, int hMax);
    void begin();

    // Pitch control
    void pitchUp(int currentPitch);
    void pitchDown(int currentPitch);
    void stopPitch();

    // Rotation control
    void rotateCW(int currentHeading);
    void rotateCCW(int currentHeading);
    void stopRotation();

    void stopAll();

    // Status getters
    String getPitchStatus() const { return pitchStatus; }
    String getRotationStatus() const { return rotationStatus; }
};

#endif
