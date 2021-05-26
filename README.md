# UCA_Education_Board
Board for Education with LoRa

Version 1.0.0, September, 2019

Author: Fabien Ferrero and Trinh Le Huy

This board was developed for education to support courses on embedded software, digital and analog electronic, telecommunication, signal processing and IoT.

<img src="https://github.com/FabienFerrero/UCA_Education_Board/blob/master/Doc/Pictures/board.png">

The board is fabricated by RFThings.

# Wiring

```
 ATMega328p       LoRa RFM95W 
                   Module
 D8          <----> RST
 MISO  (D12) <----> MISO
 MOSI  (D11) <----> MOSI
 SCK   (D13) <----> CLK
 SS    (D10) <----> SEL (Chip Select)
 D3          <----> DIO0
 D7          <----> DIO1
 D6          <----> DIO2
 3.3V        <----> Vcc
 A0          <----> BT0
 A1          <----> BT1
 A2          <----> 2

 ```
 
 <img src="https://github.com/FabienFerrero/UCA_Education_Board/blob/master/Doc/Pictures/pinout_UCA.png">
 
 
# USB Driver
The board is using CH340C chip for USB to 
You may need to install the driver to use the board:
https://sparks.gogo.co.nz/ch340.html

<img src="https://github.com/FabienFerrero/UCA_Education_Board/blob/master/Doc/Pictures/usb.png">

# Board programming

Select "Arduino Pro or Pro Mini"
Select "ATMega328p (3.3V 8MHz)"
Select the port

# Schematic

The schematic of the PCB is available the Schematic section.


# License

All rights reserved. This Gerber, program and the accompanying materials are made available under the terms of the MIT License which accompanies this distribution, and is available at https://opensource.org/licenses/mit-license.php
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Maintained by Fabien Ferrero
