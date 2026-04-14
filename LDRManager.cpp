#include "LDRManager.h"

LDRManager::LDRManager() {
  // Initialize values
  valTL = valTR = valBL = valBR = 0;
  avgTop = avgBottom = avgLeft = avgRight = avgTotal = 0;
  diffVertical = diffHorizontal = 0;
}

bool LDRManager::begin() {
  // Try to initialize the ADS1115 at default I2C address (0x48)
  return ads.begin();
}

void LDRManager::update() {
  // Read raw values from ADS1115 (Channels 0 to 3)
  // Note: ADS1115 is 16-bit, so values range from 0-32767 for positive voltages.
  // We can right shift or map if we need to match the old 0-1023 range, 
  // but for tracking, raw ratios/differences work fine.
  valTL = ads.readADC_SingleEnded(0);
  valTR = ads.readADC_SingleEnded(1);
  valBL = ads.readADC_SingleEnded(2);
  valBR = ads.readADC_SingleEnded(3);

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
