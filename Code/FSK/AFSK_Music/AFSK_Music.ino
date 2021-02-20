/*
   RadioLib AFSK Imperial March Example

   This example shows how to EXECUTE ORDER 66

   Other modules that can be used for AFSK:
    - SX127x/RFM9x
    - RF69
    - SX1231
    - CC1101
    - Si443x/RFM2x

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// include the melody
#include "melody.h"

// Parameters
int freq = 865; // Carrier Frequency in Mhz, default 865MHz
int datarate = 20; // datarate in kbps, default 20KHz, min 2KHz
int dev = 1; // Frequency deviation in KHz, default 1KHz
bool mod = 0; // 0 : FSK   1 : 00K , default 0

/* Music List
 *  0: Mix
 *  1: Starwars and Darth Vader theme (Imperial March) - Star wars 
 *  2: Hedwig's theme - Harry Potter
 *  3: The legend of Zelda theme
 *  4: God Father Theme
 *  5: Game of Thrones
 *  6: Pink Panther theme
 *  7: Tetris Theme
 *  8: Theme from Mario Bros
 *  9: 5 octaves sweep up and down 
 */

int mus = 0; // Set music, default 0


int* mel=melody1; // Pointer on music table
int m =sizeof(melody1); // size of music table
 // change this to make the song slower or faster
  byte tempo = 120;
  int j=0;


// SX1276 has the following connections:
// NSS pin:   10
// DIO0 pin:  3
// RESET pin: 9
// DIO1 pin:  7
SX1276 fsk = new Module(10, 3, 9, 6);

// create AFSK client instance using the FSK module
// this requires connection to the module direct 
// input pin, here connected to Arduino pin 6
// SX127x/RFM9x:  DIO2

AFSKClient audio(&fsk, 6);

void setup() {

  delay(500);
  Serial.begin(9600); 
  pinMode(A3,INPUT);
   

  // initialize SX1276
  Serial.print(F("[SX1276] Initializing ... "));
  // carrier frequency:           434.0 MHz
  // bit rate:                    48.0 kbps
  // frequency deviation:         50.0 kHz
  // Rx bandwidth:                125.0 kHz
  // output power:                13 dBm
  // current limit:               100 mA
  // data shaping:                Gaussian, BT = 0.5
  int state = fsk.beginFSK();

  state = fsk.setFrequency(freq);
  state = fsk.setBitRate(datarate);
  state = fsk.setFrequencyDeviation(dev);
  state = fsk.setRxBandwidth(125.0);
  state = fsk.setOutputPower(13.0);
  state = fsk.setCurrentLimit(100);
  state = fsk.setDataShaping(0.5);

 
 if (mod == 1) {
 // FSK modulation can be changed to OOK
  // NOTE: When using OOK, the maximum bit rate is only 32.768 kbps!
  //       Also, data shaping changes from Gaussian filter to
  //       simple filter with cutoff frequency. Make sure to call
  //       setDataShapingOOK() to set the correct shaping!
  state = fsk.setOOK(true);
  state = fsk.setDataShapingOOK(1);
  if (state != ERR_NONE) {
    Serial.print(F("Unable to change modulation, code "));
    Serial.println(state);
    while (true);
  }
 }

  

  if(state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while(true);
  }
}

void loop() {  

  Serial.print(F("[AFSK] Executing music N°"));
  Serial.println(mus);
        
   m=music(mus);    
  
  // calculate whole note duration
  int wholenote = (60000 * 4) / tempo;

  
  // iterate over the melody
  for(int note = 0; note < m / sizeof(*mel); note += 2) {
    // calculate the duration of each note
    int noteDuration = 0;
    int divider = *(mel+note + 1);
    if(divider > 0) {
      // regular note, just proceed
      noteDuration = wholenote / divider;
    } else if(divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = wholenote / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 95% of the duration, leaving 5% as a pause
    audio.tone(*(mel+note));
    delay(noteDuration*0.95);
    audio.noTone();
    delay(noteDuration*0.05);
  }
  
  Serial.println(F("done!"));

  // wait for a second
  delay(1000);
}


int music (int i){ 
  switch (i) {

case 1:
    mel = melody1;
    tempo = 90;
    return sizeof(melody1);    
    break;
  case 2:
    mel = melody2;
    tempo = 120;
    return sizeof(melody2);
    
    break;
  case 3:
    mel = melody3;
    tempo = 90;
    return sizeof(melody3);    
    break;
    case 4:
    mel = melody4;
    tempo = 60;
    return sizeof(melody4);    
    break;
    case 5:
    mel = melody5;
    tempo = 90;
    return  sizeof(melody5);
    break;
    case 6:
    mel = melody6;
    tempo = 120;
    return  sizeof(melody6);
    break;
    case 7:
    mel = melody7;
    tempo = 180;
    return  sizeof(melody7);
    break;
    case 8:
    mel = melody8;
    tempo = 180;
    return  sizeof(melody8);
    break;
    case 9:
    mel = melody9;
    tempo = 90;
    return  sizeof(melody9);
    break;
    case 0:
    mel = melody0;
    tempo = 120;
    return  sizeof(melody0);
    break;
    default:
    mel = melody0;
    tempo = 120;
    return  sizeof(melody0);
    break;
  }
}
