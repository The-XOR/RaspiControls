# RaspiControls
Control classes for raspberry GPIO, needs the pigpio library
http://abyz.me.uk/rpi/pigpio/

If you can't sudo... watch elsewhere.

GPIO Connections:
 
 ENCODER:
 PIN        GPIO
 ---------------------------
 GND        GND
 VCC        +3.3v
 DT         GPIO 20
 CLK        GPIO 21
 SW         GPIO 12

Led:  GPIO 24 with a 220OHM resistor


Oled: I2C, 128x64 (current address is 3C)