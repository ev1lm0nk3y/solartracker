# Solar Tracker Wiring Guide

This document provides a detailed guide for wiring all the components of the solar tracker project.

**IMPORTANT: Always disconnect power before making any changes to your wiring.**

---

## 1. Main Assembly

1.  **Arduino Motor Shield:** The Arduino Motor Shield is designed to be stacked directly on top of the Arduino Micro. Align all the pins and press it firmly into place. All connections to the motors and sensors will be made to the pins on the Motor Shield.

---

## 2. Power Connections

This project will be powered by a **12V DC Portable Battery** (which is recharged by your 100W solar panel).

1.  **Connect Portable Battery to Motor Shield (for 12V Motors):**
    *   Connect the **12V Positive (+)** output from your portable battery (e.g., from its barrel jack or cigarette lighter port, adapted to bare wires) to the **`Vin`** screw terminal on the Motor Shield.
    *   Connect the **12V Ground (-)** output from your portable battery to the **`GND`** screw terminal next to `Vin` on the Motor Shield.
    *   This supplies 12V directly to power your motors (Motor A and Motor B).

2.  **Connect Portable Battery to DC-DC Buck Converter (for 5V Electronics):**
    *   Take the same **12V Positive (+)** and **12V Ground (-)** connections from your portable battery and connect them to the input terminals of a **DC-DC Buck Converter (12V to 5V)**.
    *   Connect the **5V Positive (+)** output from the Buck Converter to the **`5V` pin** on your Arduino Micro (often accessible via the Motor Shield's header pins).
    *   Connect the **5V Ground (-)** output from the Buck Converter to a **`GND` pin** on your Arduino Micro/Motor Shield.
    *   This provides a clean, regulated 5V for the Arduino microcontroller, LCD, and compass.

3.  **Crucial Step: Remove the `Vin Connect` Jumper:**
    *   On the Arduino Motor Shield, locate the small jumper labeled **`Vin Connect`**. You **MUST remove this jumper**.
    *   Removing this jumper isolates the 12V motor power supply from the 5V power supply to the Arduino's logic. This prevents electrical noise from the motors from affecting the sensitive electronics and ensures the Arduino is powered safely by the 5V buck converter.

### **Power Flow Diagram (Conceptual)**

```
+------------------+     12V       +--------------------------+
|  12V Portable    +-------------->|  Arduino Motor Shield    |-----> 12V Motors
|  Battery Pack    |               |  (Vin & GND terminals)   |
+------------------+               +--------------------------+
       |   12V                             |
       |                                   |  (Remove Vin Connect Jumper)
       V                                   V
+------------------+         5V        +--------------------------+
|  DC-DC Buck      +------------------->|  Arduino (5V & GND pins) |-----> LCD, Compass, Arduino Logic
|  Converter       |                     +--------------------------+
| (12V Input, 5V   |
|  Output)         |
+------------------+
```

---

## 3. Motor Connections

1.  **Rotation Motor (Screw Drive):**
    *   Connect the two wires from your rotation motor to the **`Motor A`** screw terminals on the Motor Shield. Polarity doesn't matter at this stage; if the motor runs backward, you can reverse the wires or change the direction in the code.
2.  **Pitch Motor (Linear Actuator):**
    *   Connect the two wires from your pitch motor/actuator to the **`Motor B`** screw terminals on the Motor Shield.

---

## 4. I2C Devices (LCD and Compass)

The I2C protocol allows multiple devices to share the same two wires (SDA and SCL). You will connect the LCD and the compass in parallel to the Arduino's I2C pins.

*   **Arduino Micro I2C Pins:**
    *   **SDA:** Digital Pin 2
    *   **SCL:** Digital Pin 3

1.  **GY-271 (QMC5883L) Compass Connections:**
    *   `VCC` pin -> **`5V`** on the Motor Shield
    *   `GND` pin -> **`GND`** on the Motor Shield
    *   `SDA` pin -> **`D2`** (SDA) on the Motor Shield
    *   `SCL` pin -> **`D3`** (SCL) on the Motor Shield

2.  **I2C 20x4 LCD Connections:**
    *   `VCC` pin -> **`5V`** on the Motor Shield
    *   `GND` pin -> **`GND`** on the Motor Shield
    *   `SDA` pin -> **`D2`** (SDA) on the Motor Shield (connect to the same row as the compass SDA)
    *   `SCL` pin -> **`D3`** (SCL) on the Motor Shield (connect to the same row as the compass SCL)

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

