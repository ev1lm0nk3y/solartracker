# GEMINI.md

## Project Overview

This project is a sun tracker apparatus designed to keep portable solar panels optimally aligned with the sun. The system uses a combination of mechanical and electronic components to achieve this.

The mechanical assembly consists of a "lazy susan" base for rotation, driven by a screw drive motor, and a 12VDC linear actuator to control the pitch (vertical angle) of the solar panels.

The electronic control system is based on an Arduino Uno R4 WiFi controller. It uses an Arduino Motor Shield Rev3 to drive both the rotation and pitch motors. Four light detectors provide the necessary input for the sun's position. A 20x4 I2C LCD displays diagnostic information, and a GY-271 (QMC5883L) compass provides heading data.

## Building and Running

This is an Arduino-based project. The standard workflow for building and running the code is as follows:

1.  **Install the Arduino IDE:** Download and install the Arduino IDE from the official website (https://www.arduino.cc/en/software).
2.  **Install Libraries:** Install the required libraries using the Arduino IDE's Library Manager (Sketch > Include Library > Manage Libraries...).
    *   `LiquidCrystal_I2C`: Search for and install the "LiquidCrystal_I2C" library by Frank de Brabander.
    *   `QMC5883L`: This library may not be in the default library manager. You can install it by downloading the ZIP file from a source like GitHub (e.g., from user `mprograms`) and then in the IDE, go to Sketch > Include Library > Add .ZIP Library.
3.  **Compile:** Open the main `.ino` file in the Arduino IDE and click the "Verify" button to compile the code. This will check for any syntax errors.
4.  **Upload:** Connect the Arduino Micro to your computer via USB. Select the correct board and port in the Arduino IDE's "Tools" menu. Then, click the "Upload" button to flash the compiled code to the microcontroller.

## Development Conventions

*   **Code Style:** Follow the standard Arduino C++ coding conventions.
*   **File Structure:** The main sketch file should have a `.ino` extension. Additional code can be organized into separate `.h` and `.cpp` files for better modularity.
*   **Libraries:** Any external libraries used in the project should be documented in this file.
    *   `Wire.h` (for I2C communication)
    -   `LiquidCrystal_I2C.h` (for the LCD)
    -   `QMC5883L.h` (for the compass)
