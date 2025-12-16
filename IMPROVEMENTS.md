# Project Improvements & Future Features

This document tracks potential improvements and future features for the solar tracker project. These can be implemented after the core functionality is stable and working.

---

### Hardware & Safety Enhancements

- [ ] **Implement Limit Switches:**
  - **Task:** Add physical microswitches at the end-of-travel points for both the rotation (horizontal) and pitch (vertical) axes.
  - **Benefit:** Prevents motors from damaging the mechanical structure or burning out by trying to move past the physical limits. The Arduino can use these switches as digital inputs to know when to stop a motor.

- [ ] **Build a Weatherproof Enclosure:**
  - **Task:** Design and build an enclosure for the Arduino, Motor Shield, and all exposed wiring.
  - **Benefit:** Protects all electronics from rain, dew, dust, and temperature swings, ensuring long-term reliability outdoors.

- [ ] **Add a Main Fuse:**
  - **Task:** Install an in-line fuse holder on the main 12V positive (+) wire coming from the portable battery.
  - **Benefit:** Critical safety feature. A fuse (e.g., 5A or 10A, depending on motor load) will blow during a short circuit, protecting all components from catastrophic failure and reducing fire risk.

---

### Software & Logic Enhancements

- [ ] **"Return to East" Night Routine:**
  - **Task:** Modify the `shutdownRoutine()` or create a new function that, after the sun sets, uses the compass to rotate the panel to face East.
  - **Benefit:** Positions the panel to catch the first available light at sunrise, maximizing the energy generation for the next day.

- [ ] **LDR Calibration Function:**
  - **Task:** Implement a `calibrateSensors()` function that runs once during `setup()`. This function would read all four LDRs under the same ambient light and store offset values. The main `loop()` would then apply these offsets to normalize the sensor readings.
  - **Benefit:** Accounts for minor manufacturing differences between LDRs, leading to more accurate tracking.

- [ ] **Wind Protection (Advanced):**
  - **Task:** Integrate an anemometer (wind sensor). Write a "safe mode" in the `loop()` that continuously checks the wind speed. If it exceeds a set threshold, the code should override the tracking logic and move the panel to the safest position (usually flat).
  - **Benefit:** Protects the solar panel and mechanical parts from damage in high winds, where the panel can act like a sail.
