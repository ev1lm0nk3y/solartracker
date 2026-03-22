#ifndef LDR_MANAGER_H
#define LDR_MANAGER_H

#include <Arduino.h>

class LDRManager {
  private:
    uint8_t pinTL, pinTR, pinBL, pinBR;
    int valTL, valTR, valBL, valBR;
    int avgTop, avgBottom, avgLeft, avgRight, avgTotal;
    int diffVertical, diffHorizontal;

  public:
    LDRManager(uint8_t tl, uint8_t tr, uint8_t bl, uint8_t br);
    
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
