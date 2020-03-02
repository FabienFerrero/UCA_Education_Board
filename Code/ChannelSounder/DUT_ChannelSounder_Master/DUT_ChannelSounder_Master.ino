/* 
 *  
 *  Copyright (c) 2018 Fabien Ferrero
 *
 *  Permission is hereby granted, free of charge, to anyone
 *  obtaining a copy of this document and accompanying files,
 *  to do whatever they want with them without any restriction,
 *  including, but not limited to, copying, modification and redistribution.
 *  NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 *  This example receive LoRa packet with frequency hooping information in the
 *  payload and change the frequency for the next packet.
 *  This code made for Heltec LoRa board 860-915MHz version
 *  It can be use to measure LoRa device  radiation performance and to tune the 
 *  device antenna
 *  
 */

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <SPI.h>
#include <LoRa.h>
#define SS 10
#define RST 8
#define DI0 3
#define BAND 868E6
#define spreadingFactor 7
#define SignalBandwidth 125E3
//#define SignalBandwidth 31.25E3
#define preambleLength 8
#define codingRateDenominator 8


long freq = 868e6; // Default frequency at initialisation
long freq_max=870e6;
long freq_min=863e6;

long max_freq=freq; // freq with max RSSI
long min_freq=freq; // freq with max RSSI

bool Pushdetected = false; // reset peak freq

int counter = -1; // number of received packet since the last init
int index = -1;
int RSSI = -140;
int max_RSSI=-140;
int min_RSSI=0;
int average_RSSI=0;
int RSSI_array[20];
int offset = 120; // offset for RSSI
int norm = 10; // Normalization value
int buff = 20; // Buffer size for measurement
float sqDevSum=0;
int mode = 0; //define the mode used :  0:channel sounding 1:live antenna demo 

// LED control
#include <FastLED.h>
#define LED_PIN     4
#define NUM_LEDS    9
#define BRIGHTNESS  64
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
 float saturation = 1; // Between 0 and 1 (0 = gray, 1 = full color)
 float brightness = .05; // Between 0 and 1 (0 = dark, 1 is full brightness)



void setColor(int redValue,  int blueValue, int greenValue) {
fill_solid( leds, NUM_LEDS, CRGB(greenValue,redValue,blueValue));

}


void rssi2led(int RSSI){

  int LED = (RSSI+offset)/norm;
  int hue = 360 - ((RSSI+offset)*36/norm);

  long color = HSBtoRGB(hue, saturation, brightness);
 
 // Get the red, blue and green parts from generated color
 int red = color >> 16 & 255;
 int green = color >> 8 & 255;
 int blue = color & 255;
 setColor(red, blue, green);  
  
  fill_solid( leds, LED, CRGB(32,32,32));
  
  int dec = -RSSI%10;
  //Serial.println(dec);

  if (dec >5) {
    fill_solid( leds, 1, CRGB(0,32,0));
    }
  else {
    fill_solid( leds, 1, CRGB(0,0,0));
    }
    
  FastLED.show(); 

  }


void setup() { 
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);

  pinMode(2, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    
  Serial.println("LoRa Receiver");
  setColor(64, 0, 0); //GREEN  
 FastLED.show(); 
  LoRa.setPins(SS,RST,DI0);

  if (!LoRa.begin(BAND)) {
    setColor(0, 0, 64); //GREEN
    FastLED.show(); 
    while (1);
  }
  
  Serial.println("LoRa Init OK");  
  Serial.print("LoRa Frequency: ");
  Serial.println(BAND);  
  Serial.print("LoRa Spreading Factor: ");
  Serial.println(spreadingFactor);
  LoRa.setSpreadingFactor(spreadingFactor);
  
  Serial.print("LoRa Signal Bandwidth: ");
  Serial.println(SignalBandwidth);
  LoRa.setSignalBandwidth(SignalBandwidth);

  LoRa.setCodingRate4(codingRateDenominator);
   
}

void loop() {

  if (digitalRead(A0)==LOW || digitalRead(A1)==LOW|| digitalRead(2)==LOW) { 
    Pushdetected = true;    
    }

   
  
  if(Pushdetected){ // reset Peak freq and max RSSI value
    Pushdetected = false;
    counter = -1; // start with count < 0 to add a small delay before the start of the measurement
    index = 0;
    if(mode ==2){
      mode = 0;
      Serial.println("Mode 0");
      offset = 120;
      buff = 20;
    norm = 10;
    setColor(32, 00, 0); //GREEN  
    FastLED.show(); 
    delay (1000);
    }
    else if (mode==1){
    mode = 2; 
    Serial.println("Mode 2"); 
    offset = 80;
    norm = 5;
    buff = 1;
    setColor(00, 32, 00); //Blue  
    FastLED.show(); 
    delay (1000);
    }
    
    else {
    mode = 1;
    Serial.println("Mode 1");  
    offset = 70;
    norm = 5;
    buff = 2;
    setColor(00, 00, 32); //RED  
    FastLED.show(); 
    delay (1000);
      }
    }
  
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packets
  
    
        
    counter++;
    if (index<buff){
    index++;}

    if (counter >buff) { // reset array
      counter = 0;
      index = buff;
      }
    
    // read packet
    while (LoRa.available()) {
      String data = LoRa.readString();
      freq = data.toInt(); // data converted to int
     // If a packet is not received with a suitable frequency, set in the good freq range
     if(freq > freq_max){
     freq = freq_max;
     }
     if(freq < freq_min){
     freq = freq_min;
     }
     
    //RSSI calculation
      RSSI = LoRa.packetRssi()+157; // Extract raw value of packetrssi register
      RSSI = (16*RSSI)/15; //value correction from the datasheet
      RSSI = RSSI -157; // Re-Calculate value in dBm

      if(counter >= 0){
      RSSI_array[counter]=RSSI;
      
      }
      
    long freq_MHz= freq / 1e6; // freq in MHz 

    
     if(index > (buff-1)){

    average_RSSI = 0;
    for( int i=0; i<=index; i++ ) {
    average_RSSI = average_RSSI + RSSI_array[i];
      }
    average_RSSI = average_RSSI / (index+1);
    
    sqDevSum=0;
    for( int i=0; i<=counter; i++ ) {
      sqDevSum += (float) pow(average_RSSI - RSSI_array[i], 2);
    
      }
    sqDevSum = sqrt(sqDevSum /counter);

    

     if(RSSI > max_RSSI && counter > 0){
      max_RSSI=RSSI;
      max_freq=freq;
      }

  if(RSSI < min_RSSI && counter > 0){
      min_RSSI=RSSI;
      min_freq=freq;
      }    
    // print RSSI of packet
           
      Serial.print(freq);
      long minfreq_MHz= min_freq / 1e6; // freq in MHz

    Serial.print("  ");
    Serial.print(RSSI);
    Serial.print("  ");
    Serial.println(average_RSSI);
    rssi2led(average_RSSI);
      
      }
    }    
    long maxfreq_MHz= max_freq / 1e6; // freq in MHz
    
    LoRa.setFrequency(freq); // change frequency
  }
}

long HSBtoRGB(float _hue, float _sat, float _brightness) {
   float red = 0.0;
   float green = 0.0;
   float blue = 0.0;
   
   if (_sat == 0.0) {
       red = _brightness;
       green = _brightness;
       blue = _brightness;
   } else {
       if (_hue == 360.0) {
           _hue = 0;
       }

       int slice = _hue / 60.0;
       float hue_frac = (_hue / 60.0) - slice;

       float aa = _brightness * (1.0 - _sat);
       float bb = _brightness * (1.0 - _sat * hue_frac);
       float cc = _brightness * (1.0 - _sat * (1.0 - hue_frac));
       
       switch(slice) {
           case 0:
               red = _brightness;
               green = cc;
               blue = aa;
               break;
           case 1:
               red = bb;
               green = _brightness;
               blue = aa;
               break;
           case 2:
               red = aa;
               green = _brightness;
               blue = cc;
               break;
           case 3:
               red = aa;
               green = bb;
               blue = _brightness;
               break;
           case 4:
               red = cc;
               green = aa;
               blue = _brightness;
               break;
           case 5:
               red = _brightness;
               green = aa;
               blue = bb;
               break;
           default:
               red = 0.0;
               green = 0.0;
               blue = 0.0;
               break;
       }
   }

   long ired = red * 255.0;
   long igreen = green * 255.0;
   long iblue = blue * 255.0;
   
   return long((ired << 16) | (igreen << 8) | (iblue));
}
