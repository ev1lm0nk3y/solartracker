# Solar Tracker Wiring Guide

This document provides a detailed guide for wiring all the components of the solar tracker project.

**IMPORTANT: Always disconnect power before making any changes to your wiring.**

---

## 1. Main Assembly

1.  **Arduino Motor Shield:** The Arduino Motor Shield Rev3 is designed to stack directly on top of the Arduino Uno R4 WiFi. Align the pins and press it firmly down so the shield sits on top of the Uno.

---

## 2. Power Connections

This project will be powered by a **12V DC Portable Battery** (recharged by your 100W solar panel).

1.  **Connect Portable Battery to Motor Shield (for 12V Motors):**
    *   Connect the **12V Positive (+)** from your battery to the **`Vin`** screw terminal on the Motor Shield.
    *   Connect the **12V Ground (-)** from your battery to the **`GND`** screw terminal on the Motor Shield.

2.  **Powering the Arduino Uno R4 WiFi:**
    *   **Option A (Recommended for isolation):** Remove the **`Vin Connect`** jumper on the Motor Shield. Use a separate power source for the Arduino (like the USB-C port connected to a 5V power bank or your DC-DC converter outputting 5V to the Arduino's 5V/GND pins). This isolates the microcontroller from motor electrical noise.
    *   **Option B (Simplest):** Keep the **`Vin Connect`** jumper in place. The 12V connected to the shield will also power the Arduino Uno R4 WiFi (which supports up to 24V input). *Note: If you experience the Arduino resetting when motors move, switch to Option A.*

---

## 3. Motor Connections

1.  **Rotation Motor (Screw Drive):**
    *   Connect wires to **`Motor A`** terminals on the Shield.
2.  **Pitch Motor (Linear Actuator):**
    *   Connect wires to **`Motor B`** terminals on the Shield.

---

## 4. I2C Devices (LCD and Compass)

The I2C bus uses the SDA and SCL pins. On the Arduino Uno R4 WiFi, these are located on the header near the USB connector / Reset button.

1.  **GY-271 (QMC5883L) Compass:**
    *   `VCC` -> **`5V`**
    *   `GND` -> **`GND`**
    *   `SDA` -> **`SDA`** pin (or A4)
    *   `SCL` -> **`SCL`** pin (or A5)

2.  **MPU6050 Accelerometer (GY-521):**
    *   **Purpose:** Measures the pitch (tilt) angle to stop the motor before mechanical limits.
    *   **Mounting:** Mount this sensor **flat** on the back of your solar panel frame.
    *   `VCC` -> **`5V`**
    *   `GND` -> **`GND`**
    *   `SDA` -> **`SDA`** pin (same as compass)
    *   `SCL` -> **`SCL`** pin (same as compass)

3.  **I2C 20x4 LCD:**
    *   `VCC` -> **`5V`**
    *   `GND` -> **`GND`**
    *   `SDA` -> **`SDA`** pin (same as compass)
    *   `SCL` -> **`SCL`** pin (same as compass)

---

## 5. Light Sensors (LDRs)

Each of the four LDRs needs to be set up in a **voltage divider** circuit with a 10kΩ resistor. This allows the Arduino to read the change in resistance as a change in voltage.

You will need **four 10kΩ resistors**.

Repeat the following steps for each of the four LDRs:

#### LDR 1: Top-Left (TL)
1.  Connect one leg of the LDR to **`5V`** on the Motor Shield.
2.  Connect the other leg of the LDR to a row on your breadboard.
3.  In the same row, connect one leg of a 10kΩ resistor.
4.  Also in the same row, connect a wire that goes to analog pin **`A0`** on the Motor Shield.
5.  Connect the other leg of the 10kΩ resistor to **`GND`** on the Motor Shield.

#### LDR 2: Top-Right (TR)
*   Repeat the process above, but connect the signal wire to analog pin **`A1`**.

#### LDR 3: Bottom-Left (BL)
*   Repeat the process, but connect the signal wire to analog pin **`A2`**.

#### LDR 4: Bottom-Right (BR)
*   Repeat the process, but connect the signal wire to analog pin **`A3`**.


### Voltage Divider Diagram (Text):

```
      (5V)
        |
       /
      /  LDR
     /
      |
      +------> to Analog Pin (A0, A1, A2, or A3)
      |
      \
      /  10kΩ Resistor
      \
      |
     (GND)
```

