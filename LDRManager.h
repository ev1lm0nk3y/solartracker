#ifndef LDR_MANAGER_H
#define LDR_MANAGER_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

class LDRManager {
  private:
    Adafruit_ADS1115 ads;
    int valTL, valTR, valBL, valBR;
    int avgTop, avgBottom, avgLeft, avgRight, avgTotal;
    int diffVertical, diffHorizontal;

  public:
    LDRManager();
    
    // Initialize the ADS1115
    bool begin();

    // Reads sensors and performs calculations
    void update();

    // Get individual raw values
    int getTopLeft() const;
    int getTopRight() const;
    int getBottomLeft() const;
    int getBottomRight() const;

    // Get calculated averages
    int getTopAverage() const;
    int getBottomAverage() const;
    int getLeftAverage() const;
    int getRightAverage() const;
    int getTotalAverage() const;

    // Get calculated differences
    int getVerticalDiff() const;
    int getHorizontalDiff() const;
};

#endif
