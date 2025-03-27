# esp32fw
ESP32 based filter wheel automation

Simple firmware to automate a filter wheel operation by adding a motor and web/INDI interfaces.
Homing is based on a magnet, hall sensor combination. My implementation uses a linear sensor making the detection a lot more complex than using a digital one (the expectation is that precision is better).
INDI driver is included. Instructions on how to compile it can be found in INDI's website (https://docs.indilib.org/drivers/).
