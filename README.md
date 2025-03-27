# esp32fw
ESP32 based filter wheel automation

- Simple firmware to automate a filter wheel operation by adding a motor and web/INDI interfaces.
- Main program is intended for a single core ESP32 (e.g. ESP32-C3) and uses FreeRTOS tasks.
- Homing is based on a magnet / hall sensor combination. My implementation uses a linear sensor making the detection a lot more complex than using a digital one (the expectation is that precision is better, but I used it because it was what was at hand).
- INDI driver is included. Instructions on how to compile it can be found in INDI's website (https://docs.indilib.org/drivers/).

Features

- INDI support
- Simple web interface (could be used by an Android app, for example)
- Comms run over IP only
- Example HW implementation uses a 28BYJ-48 (5v) stepper which can be successfully powered by a RPi USB port
- 3D printed case and pulley for SVBony manual filter wheel (SV133), ESP32-C3-mini, 28BYJ-48 stepper and ULN2003 driver included (OpenSCAD file)

To do

- Autocalibration routine
- Homing optimizations (current method just goes one full turn to find magnet)
- Android app
