# DDS-AD9912-Arduino-Shield
DDS (Direct Digital Synthesis) Analog Devices AD9912 Arduino Shield by GRA &amp; AFCH

Web-site: https://gra-afch.com
Direct link to category:  https://gra-afch.com/product-category/rf-units/

# GA_Flasher can be used to upload compiled firmware from "Firmware Compiled (.HEX File)" folder: https://github.com/afch/GA_Flasher

"Firmware Compiled (.HEX File)" - folder contains pre-Compiled firmware (.HEX File), that can be uploaded via "AvrDude", GA_Flasher or any other software for flashing Atmega 2560

"Firmware Source (.INO File)" - folder contains source of firmware that can be compiled (modified if necessary) and uploaded via Arduino IDE: https://www.arduino.cc/en/Main/Software

"Libraries" - contains libraries that are only needed for compilation: place them to "C:\Users\[USER]\Documents\Arduino\libraries", where [USER] - your windows username.

This AD9912 Shield can be easily connected to Arduino Mega without additional wires and converters. 

# The switching of clock sources is performed by the following components:
<pre>
|-------------------------------------|-----------------------------------|-------------------------------|--------------|
|      Clock source                   |            Capacitors             |    Resistors                  | Ferrite Bead |
|   (only one at a time)              |  C20  | C18,C19 | C14,C17 |  C15  |  XO  | REF | PLL_BP(1k) | R37 |      FB1     |
|------------------------------------------------------------------------------------------------------------------------|
| XO - Crystal Oscillator (Z1)        |   -   |    V    |    X    |   V   |   V  |  X  |      X     |  V  |       X      |
| TCXO - Oscillator 1ppm (Z2)         |   V   |    X    |    V    |   V   |   X  |  V  |      X     |  V  |       V      |
| OCXO - Oscillator 0.1ppm (Z3)       |   X   |    X    |    V    |   V   |   X  |  V  |      X     |  V  |       V      |
| REF CLK IN - External Generator     |   X   |    X    |    V    |   X   |   X  |  V  |      V     |  X  |       X      |
|------------------------------------------------------------------------------------------------------------------------|
</pre>

Where V means that the component must be installed (soldered), and X - means that the component must be removed
