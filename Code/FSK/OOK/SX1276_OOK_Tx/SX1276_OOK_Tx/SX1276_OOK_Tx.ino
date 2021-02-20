/*
   The exemple is used to show a simple FSK modulation

   Parameters are set to easily understand the FSK modulation on a SDR spectrogram

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// Parameters

int freq = 865; // Center Frequency

// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
SX1276 radio = new Module(10, 3, 8, 6);

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1278 fsk = RadioShield.ModuleA;

void setup() {
  Serial.begin(9600);

  // initialize SX1278 FSK modem with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  int state = radio.beginFSK();
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // if needed, you can switch between LoRa and FSK modes
  //
  // radio.begin()       start LoRa mode (and disable FSK)
  // radio.beginFSK()    start FSK mode (and disable LoRa)

  // the following settings can also
  // be modified at run-time
  state = radio.setFrequency(freq);
  state = radio.setBitRate(4.0);
  state = radio.setFrequencyDeviation(100.0);
  state = radio.setRxBandwidth(250.0);
  state = radio.setOutputPower(10.0);
  state = radio.setCurrentLimit(100);
  state = radio.setDataShaping(RADIOLIB_SHAPING_0_5);
  uint8_t syncWord[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
  state = radio.setSyncWord(syncWord, 8);
  if (state != ERR_NONE) {
    Serial.print(F("Unable to set configuration, code "));
    Serial.println(state);
    while (true);
  }

//  // FSK modulation can be changed to OOK
//  // NOTE: When using OOK, the maximum bit rate is only 32.768 kbps!
//  //       Also, data shaping changes from Gaussian filter to
//  //       simple filter with cutoff frequency. Make sure to call
//  //       setDataShapingOOK() to set the correct shaping!
 state = radio.setOOK(true);
 state = radio.setDataShapingOOK(1);
  if (state != ERR_NONE) {
    Serial.print(F("Unable to change modulation, code "));
   Serial.println(state);
   while (true);
  }

  #warning "This sketch is just an API guide! Read the note at line 6."
}

void loop() {
  // FSK modem can use the same transmit/receive methods
  // as the LoRa modem, even their interrupt-driven versions
  // NOTE: FSK modem maximum packet length is 63 bytes!

  // transmit FSK packet
  int state = radio.transmit("Hello World! this is me");
  
  if (state == ERR_NONE) {
    Serial.println(F("[SX1278] Packet transmitted successfully!"));
  } else if (state == ERR_PACKET_TOO_LONG) {
    Serial.println(F("[SX1278] Packet too long!"));
  } else if (state == ERR_TX_TIMEOUT) {
    Serial.println(F("[SX1278] Timed out while transmitting!"));
  } else {
    Serial.println(F("[SX1278] Failed to transmit packet, code "));
    Serial.println(state);
  }

 
}
