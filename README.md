# UCA_Education_Board
Board for Education with LoRa

Version 1.0.0, September, 2019

Author: Fabien Ferrero, Trinh Le Huy

This board was developed for education to support courses on embedded software, digital and analog electronic, telecommunication, signal processing and IoT.

# Wiring

```
 ATMega328p       LoRa RFM95W 
                   Module
 D8          <----> RST
 MISO  (D12) <----> MISO
 MOSI  (D11) <----> MOSI
 SCK   (D13) <----> CLK
 SS    (D10) <----> SEL (Chip Select)
 D2          <----> DIO0
 D7          <----> DIO1
 D9          <----> DIO2
 3.3V        <----> Vcc

 ```

# Schematic

The schematic of the PCB is available the Schematic section.


# License

All rights reserved. This Gerber, program and the accompanying materials are made available under the terms of the MIT License which accompanies this distribution, and is available at https://opensource.org/licenses/mit-license.php
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Maintained by Fabien Ferrero
