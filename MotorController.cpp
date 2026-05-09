#include "MotorController.h"

MotorController::MotorController(uint8_t dirA, uint8_t pwmA, uint8_t brakeA,
                                 uint8_t dirB, uint8_t pwmB, uint8_t brakeB,
                                 int speed)
    : pinDirA(dirA), pinPwmA(pwmA), pinBrakeA(brakeA),
      pinDirB(dirB), pinPwmB(pwmB), pinBrakeB(brakeB),
      motorSpeed(speed), pitchStatus("STOP"), rotationStatus("STOP") {}

void MotorController::setLimits(int pMin, int pMax, int hMin, int hMax) {
    pitchMin = pMin;
    pitchMax = pMax;
    headingMin = hMin;
    headingMax = hMax;
}

void MotorController::begin() {
    pinMode(pinDirA, OUTPUT);
    pinMode(pinPwmA, OUTPUT);
    pinMode(pinBrakeA, OUTPUT);
    digitalWrite(pinBrakeA, LOW);

    pinMode(pinDirB, OUTPUT);
    pinMode(pinPwmB, OUTPUT);
    pinMode(pinBrakeB, OUTPUT);
    digitalWrite(pinBrakeB, LOW);
}

void MotorController::pitchUp(int currentPitch) {
    if (currentPitch >= pitchMax) {
        stopPitch();
        pitchStatus = "MAX";
        return;
    }
    digitalWrite(pinDirB, HIGH);
    analogWrite(pinPwmB, motorSpeed);
    pitchStatus = "UP";
}

void MotorController::pitchDown(int currentPitch) {
    if (currentPitch <= pitchMin) {
        stopPitch();
        pitchStatus = "MIN";
        return;
    }
    digitalWrite(pinDirB, LOW);
    analogWrite(pinPwmB, motorSpeed);
    pitchStatus = "DOWN";
}

void MotorController::stopPitch() {
    analogWrite(pinPwmB, 0);
    pitchStatus = "STOP";
}

void MotorController::rotateCW(int currentHeading) {
    if (currentHeading >= headingMax) {
        stopRotation();
        rotationStatus = "MAX";
        return;
    }
    digitalWrite(pinDirA, HIGH);
    analogWrite(pinPwmA, motorSpeed);
    rotationStatus = "CW";
}

void MotorController::rotateCCW(int currentHeading) {
    if (currentHeading <= headingMin) {
        stopRotation();
        rotationStatus = "MIN";
        return;
    }
    digitalWrite(pinDirA, LOW);
    analogWrite(pinPwmA, motorSpeed);
    rotationStatus = "CCW";
}

void MotorController::stopRotation() {
    analogWrite(pinPwmA, 0);
    rotationStatus = "STOP";
}

void MotorController::stopAll() {
    stopPitch();
    stopRotation();
}
