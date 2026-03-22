#include "LDRManager.h"

LDRManager::LDRManager(uint8_t tl, uint8_t tr, uint8_t bl, uint8_t br) {
  pinTL = tl;
  pinTR = tr;
  pinBL = bl;
  pinBR = br;
  
  // Initialize values
  valTL = valTR = valBL = valBR = 0;
  avgTop = avgBottom = avgLeft = avgRight = avgTotal = 0;
  diffVertical = diffHorizontal = 0;
  
  // Pins are analog inputs, no pinMode needed for analogRead on most Arduinos,
  // but explicit input mode doesn't hurt.
  pinMode(pinTL, INPUT);
  pinMode(pinTR, INPUT);
  pinMode(pinBL, INPUT);
  pinMode(pinBR, INPUT);
}

void LDRManager::update() {
  // Read raw values
  valTL = analogRead(pinTL);
  valTR = analogRead(pinTR);
  valBL = analogRead(pinBL);
  valBR = analogRead(pinBR);

  // Calculate Averages
  avgTop = (valTL + valTR) / 2;
  avgBottom = (valBL + valBR) / 2;
  avgLeft = (valTL + valBL) / 2;
  avgRight = (valTR + valBR) / 2;
  avgTotal = (valTL + valTR + valBL + valBR) / 4;

  // Calculate Differences
  diffVertical = avgTop - avgBottom;
  diffHorizontal = avgLeft - avgRight;
}

int LDRManager::getTopLeft() const { return valTL; }
int LDRManager::getTopRight() const { return valTR; }
int LDRManager::getBottomLeft() const { return valBL; }
int LDRManager::getBottomRight() const { return valBR; }

int LDRManager::getTopAverage() const { return avgTop; }
int LDRManager::getBottomAverage() const { return avgBottom; }
int LDRManager::getLeftAverage() const { return avgLeft; }
int LDRManager::getRightAverage() const { return avgRight; }
int LDRManager::getTotalAverage() const { return avgTotal; }

int LDRManager::getVerticalDiff() const { return diffVertical; }
int LDRManager::getHorizontalDiff() const { return diffHorizontal; }
