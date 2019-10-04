/*
 *  temperature sensor on analog A0 to test the LoRa gateway
 *  extended version with AES and custom Carrier Sense features
 *  
 *  Copyright (C) 2016-2019 Congduc Pham, University of Pau, France
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 * last update: May 21th, 2019 by C. Pham
 * 
 * This version uses the same structure than the Arduino_LoRa_Demo_Sensor where
 * the sensor-related code is in a separate file
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// add here some specific board define statements if you want to implement user-defined specific settings
// A/ LoRa radio node from IoTMCU: https://www.tindie.com/products/IOTMCU/lora-radio-node-v10/
//#define IOTMCU_LORA_RADIO_NODE
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h> 
// Include the SX1272
#include "SX1272.h"
#include "my_temp_sensor_code.h"

/********************************************************************
 _____              __ _                       _   _             
/  __ \            / _(_)                     | | (_)            
| /  \/ ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ _  ___  _ __  
| |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \ 
| \__/\ (_) | | | | | | | (_| | |_| | | | (_| | |_| | (_) | | | |
 \____/\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
                          __/ |                                  
                         |___/                                   
********************************************************************/

///////////////////////////////////////////////////////////////////
// COMMENT OR UNCOMMENT TO CHANGE FEATURES. 
// ONLY IF YOU KNOW WHAT YOU ARE DOING!!! OTHERWISE LEAVE AS IT IS
// 
// FOR SIMPLY DOING AES ENCRYTION: ENABLE AES
// FOR TTN WITHOUT LORAWAN: ENABLE WITH_AES & EXTDEVADDR 
// FOR TTN WITH LORAWAN: ENABLE WITH_AES & LORAWAN
//
//#define WITH_EEPROM
//#define WITH_APPKEY
//if you are low on program memory, comment STRING_LIB to save about 2K
#define STRING_LIB
//#define LOW_POWER
//#define LOW_POWER_HIBERNATE
#define SHOW_LOW_POWER_CYCLE
//Use LoRaWAN AES-like encryption
#define WITH_AES
//Use our Lightweight Stream Cipher (LSC) algorithm
//#define WITH_LSC
//If you want to upload on TTN without LoRaWAN you have to provide the 4 bytes DevAddr and uncomment #define EXTDEVADDR
//#define EXTDEVADDR
//Use native LoRaWAN packet format to send to LoRaWAN gateway
#define LORAWAN
//uncomment to use a customized frequency. TTN plan includes 868.1/868.3/868.5/867.1/867.3/867.5/867.7/867.9 for LoRa
//#define MY_FREQUENCY 868.1
//when sending to a LoRaWAN gateway (e.g. running util_pkt_logger) but with no native LoRaWAN format, just to set the correct sync word. DO NOT use if LORAWAN is uncommented
//#define USE_LORAWAN_SW
//#define WITH_ACK
//this will enable a receive window after every transmission
//#define WITH_RCVW
//force 20dBm, use with caution, test ony
#define USE_20DBM
// with SF12, 23+4=27 -> 1.024s
#define MY_LORA_PREAMBLE_LENGTH 23
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// ADD HERE OTHER PLATFORMS THAT DO NOT SUPPORT EEPROM NOR LOW POWER
#if defined ARDUINO_SAM_DUE || defined __SAMD21G18A__
#undef WITH_EEPROM
#endif

#if defined ARDUINO_SAM_DUE
#undef LOW_POWER
#endif
///////////////////////////////////////////////////////////////////

// IMPORTANT SETTINGS
///////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//
#define ETSI_EUROPE_REGULATION
//#define FCC_US_REGULATION
//#define SENEGAL_REGULATION
///////////////////////////////////////////////////////////////////

// IMPORTANT
///////////////////////////////////////////////////////////////////
// please edit band_config.h to define the frequency band
#include "band_config.h"
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// uncomment if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
#define PABOOST
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS BETWEEN 2 AND 255
uint8_t node_addr=6;
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE
#define LORAMODE  1
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE SPREADING FACTOR ONLY FOR LORAWAN MODE
#ifdef LORAWAN
int SF=9;
#endif
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE TIME IN MINUTES BETWEEN 2 READING & TRANSMISSION
unsigned int idlePeriodInMin = 1;
///////////////////////////////////////////////////////////////////

#ifdef WITH_APPKEY
///////////////////////////////////////////////////////////////////
// CHANGE HERE THE APPKEY, BUT IF GW CHECKS FOR APPKEY, MUST BE
// IN THE APPKEY LIST MAINTAINED BY GW.
uint8_t my_appKey[4]={5, 6, 7, 8};
///////////////////////////////////////////////////////////////////
#endif

///////////////////////////////////////////////////////////////////
// ENCRYPTION CONFIGURATION AND KEYS FOR LORAWAN
#ifdef WITH_AES
#include "local_lorawan.h"

///////////////////////////////////////////////////////////////////
//For TTN, use specify ABP mode on the TTN device setting

///////////////////////////////////////////////////////////////////
//ENTER HERE your App Session Key from the TTN device info (same order, i.e. msb)
//unsigned char AppSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
///////////////////////////////////////////////////////////////////

//Danang
unsigned char AppSkey[16] = { 0xD0, 0xCB, 0xAB, 0x62, 0x4B, 0x6B, 0x0E, 0x1B, 0xBC, 0x30, 0x17, 0x7E, 0x8F, 0xB0, 0x12, 0x61 };

//Pau
//unsigned char AppSkey[16] = { 0x05, 0x40, 0xAC, 0x07, 0xB0, 0x9E, 0x0C, 0x60, 0x65, 0x0D, 0x50, 0xCF, 0x00, 0xF0, 0x1C, 0x0D };

//this is the default as LoRaWAN example
//unsigned char AppSkey[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

///////////////////////////////////////////////////////////////////
//ENTER HERE your Network Session Key from the TTN device info (same order, i.e. msb)
//unsigned char NwkSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
///////////////////////////////////////////////////////////////////

//Danang
unsigned char NwkSkey[16] = { 0xDE, 0xF3, 0x70, 0xA2, 0xF2, 0xF3, 0x20, 0xAA, 0xF3, 0x22, 0x5C, 0x26, 0x3C, 0x10, 0x93, 0x56 };

//Pau
//unsigned char NwkSkey[16] = { 0x01, 0x10, 0xFF, 0x00, 0x60, 0xBA, 0x0A, 0xE0, 0x0F, 0x0A, 0x60, 0x6B, 0x0A, 0x50, 0x8F, 0x01 };

//this is the default as LoRaWAN example
//unsigned char NwkSkey[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

///////////////////////////////////////////////////////////////////
// LORAWAN OR EXTENDED DEVICE ADDRESS FOR LORAWAN CLOUD
#if defined LORAWAN || defined EXTDEVADDR
///////////////////////////////////////////////////////////////////
//ENTER HERE your Device Address from the TTN device info (same order, i.e. msb). Example for 0x12345678
//unsigned char DevAddr[4] = { 0x12, 0x34, 0x56, 0x78 };
///////////////////////////////////////////////////////////////////

//Danang
unsigned char DevAddr[4] = { 0x26, 0x01, 0x11, 0x82 };

//Pau
//unsigned char DevAddr[4] = { 0x26, 0x01, 0x17, 0x21 };
#else
///////////////////////////////////////////////////////////////////
// DO NOT CHANGE HERE
unsigned char DevAddr[4] = { 0x00, 0x00, 0x00, node_addr };
///////////////////////////////////////////////////////////////////
#endif
#endif

///////////////////////////////////////////////////////////////////
// ENCRYPTION CONFIGURATION AND KEYS FOR LSC ENCRYPTION METHOD
#ifdef WITH_LSC
#include "local_lsc.h"

///////////////////////////////////////////////////////////////////
//Enter here your LSC encryption key
uint8_t LSC_Nonce[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
///////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// IF YOU SEND A LONG STRING, INCREASE THE SIZE OF MESSAGE
uint8_t message[80];
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// COMMENT THIS LINE IF YOU WANT TO DYNAMICALLY SET THE NODE'S ADDR 
// OR SOME OTHER PARAMETERS BY REMOTE RADIO COMMANDS (WITH_RCVW)
// LEAVE THIS LINE UNCOMMENTED IF YOU WANT TO USE THE DEFAULT VALUE
// AND CONFIGURE YOUR DEVICE BY CHANGING MANUALLY THESE VALUES IN 
// THE SKETCH.
//
// ONCE YOU HAVE FLASHED A BOARD WITHOUT FORCE_DEFAULT_VALUE, YOU 
// WILL BE ABLE TO DYNAMICALLY CONFIGURE IT AND SAVE THIS CONFIGU-
// RATION INTO EEPROM. ON RESET, THE BOARD WILL USE THE SAVED CON-
// FIGURATION.

// IF YOU WANT TO REINITIALIZE A BOARD, YOU HAVE TO FIRST FLASH IT 
// WITH FORCE_DEFAULT_VALUE, WAIT FOR ABOUT 10s SO THAT IT CAN BOOT
// AND FLASH IT AGAIN WITHOUT FORCE_DEFAULT_VALUE. THE BOARD WILL 
// THEN USE THE DEFAULT CONFIGURATION UNTIL NEXT CONFIGURATION.

#define FORCE_DEFAULT_VALUE
///////////////////////////////////////////////////////////////////

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/                        
                        
// we wrapped Serial.println to support the Arduino Zero or M0
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(fmt,param)   SerialUSB.print(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define PRINT_HEX(fmt,param)      SerialUSB.print(param,HEX)
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define PRINT_HEX(fmt,param)      Serial.print(param,HEX)
#define FLUSHOUTPUT               Serial.flush();
#endif

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#ifdef ETSI_EUROPE_REGULATION
#define MAX_DBM 14
// previous way for setting output power
// char powerLevel='M';
#elif defined SENEGAL_REGULATION
#define MAX_DBM 10
// previous way for setting output power
// 'H' is actually 6dBm, so better to use the new way to set output power
// char powerLevel='H';
#elif defined FCC_US_REGULATION
#define MAX_DBM 14
#endif

#ifdef USE_20DBM
#undef MAX_DBM
#define MAX_DBM 20
#endif

#ifdef BAND868
#ifdef SENEGAL_REGULATION
const uint32_t DEFAULT_CHANNEL=CH_04_868;
#else
const uint32_t DEFAULT_CHANNEL=CH_10_868;
#endif
#elif defined BAND900
const uint32_t DEFAULT_CHANNEL=CH_05_900;
// For HongKong, Japan, Malaysia, Singapore, Thailand, Vietnam: 920.36MHz     
//const uint32_t DEFAULT_CHANNEL=CH_08_900;
#elif defined BAND433
const uint32_t DEFAULT_CHANNEL=CH_00_433;
#endif

#define DEFAULT_DEST_ADDR 1

#ifdef WITH_ACK
#define NB_RETRIES 2
#endif

#ifdef LOW_POWER
// this is for the Teensy36, Teensy35, Teensy31/32 & TeensyLC
// need v6 of Snooze library
#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
#define LOW_POWER_PERIOD 60
#include <Snooze.h>
SnoozeTimer timer;
SnoozeBlock sleep_config(timer);
#else // for all other boards based on ATMega168, ATMega328P, ATMega32U4, ATMega2560, ATMega256RFR2, ATSAMD21G18A
#define LOW_POWER_PERIOD 8
// you need the LowPower library from RocketScream
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"

#ifdef __SAMD21G18A__
// use the RTC library
#include "RTCZero.h"
/* Create an rtc object */
RTCZero rtc;
#endif
#endif
unsigned int nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
#endif

unsigned long nextTransmissionTime=0L;

#ifdef LORAWAN
//we first set to use BW125SF12
int loraMode=1;
#else
int loraMode=LORAMODE;
#endif

#ifdef WITH_EEPROM
struct sx1272config {

  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
  uint8_t addr;
  unsigned int idle_period;  
  uint8_t overwrite;
  // can add other fields such as LoRa mode,...
};

sx1272config my_sx1272config;
#endif

#ifdef WITH_RCVW

// will wait for 5s before opening the rcv window
#define DELAY_BEFORE_RCVW 5000

//this function is provided to parse the downlink command which is assumed to be in the format /@A6#
//
long getCmdValue(int &i, char* strBuff=NULL) {
  
    char seqStr[7]="******";
    
    int j=0;
    // character '#' will indicate end of cmd value
    while ((char)message[i]!='#' && (i < strlen((char*)message)) && j<strlen(seqStr)) {
            seqStr[j]=(char)message[i];
            i++;
            j++;
    }
    
    // put the null character at the end
    seqStr[j]='\0';
    
    if (strBuff) {
            strcpy(strBuff, seqStr);        
    }
    else
            return (atol(seqStr));
}   
#endif

#ifndef STRING_LIB

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 if (desimal < p[precision-1]) {
  *a++ = '0';
 } 
 itoa(desimal, a, 10);
 return ret;
}
#endif


/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/

void setup()
{
  int e;

  // initialization of the temperature sensor
  sensor_Init();

#ifdef LOW_POWER
#ifdef __SAMD21G18A__
  rtc.begin();
#endif  
#else
  digitalWrite(PIN_POWER,HIGH);
#endif
  
  delay(3000);
  // Open serial communications and wait for port to open:
#if defined __SAMD21G18A__ && not defined ARDUINO_SAMD_FEATHER_M0 
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif  
  // Print a start message
  PRINT_CSTSTR("%s","LoRa temperature sensor, extended version\n");

#ifdef ARDUINO_AVR_PRO
  PRINT_CSTSTR("%s","Arduino Pro Mini detected\n");  
#endif
#ifdef ARDUINO_AVR_NANO
  PRINT_CSTSTR("%s","Arduino Nano detected\n");   
#endif
#ifdef ARDUINO_AVR_MINI
  PRINT_CSTSTR("%s","Arduino MINI/Nexus detected\n");  
#endif
#ifdef ARDUINO_AVR_MEGA2560
  PRINT_CSTSTR("%s","Arduino Mega2560 detected\n");  
#endif
#ifdef ARDUINO_SAM_DUE
  PRINT_CSTSTR("%s","Arduino Due detected\n");  
#endif
#ifdef __MK66FX1M0__
  PRINT_CSTSTR("%s","Teensy36 MK66FX1M0 detected\n");
#endif
#ifdef __MK64FX512__
  PRINT_CSTSTR("%s","Teensy35 MK64FX512 detected\n");
#endif
#ifdef __MK20DX256__
  PRINT_CSTSTR("%s","Teensy31/32 MK20DX256 detected\n");
#endif
#ifdef __MKL26Z64__
  PRINT_CSTSTR("%s","TeensyLC MKL26Z64 detected\n");
#endif
#if defined ARDUINO_SAMD_ZERO && not defined ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("%s","Arduino M0/Zero detected\n");
#endif
#ifdef ARDUINO_AVR_FEATHER32U4 
  PRINT_CSTSTR("%s","Adafruit Feather32U4 detected\n"); 
#endif
#ifdef  ARDUINO_SAMD_FEATHER_M0
  PRINT_CSTSTR("%s","Adafruit FeatherM0 detected\n");
#endif

// See http://www.nongnu.org/avr-libc/user-manual/using_tools.html
// for the list of define from the AVR compiler

#ifdef __AVR_ATmega328P__
  PRINT_CSTSTR("%s","ATmega328P detected\n");
#endif 
#ifdef __AVR_ATmega32U4__
  PRINT_CSTSTR("%s","ATmega32U4 detected\n");
#endif 
#ifdef __AVR_ATmega2560__
  PRINT_CSTSTR("%s","ATmega2560 detected\n");
#endif 
#ifdef __SAMD21G18A__ 
  PRINT_CSTSTR("%s","SAMD21G18A ARM Cortex-M0+ detected\n");
#endif
#ifdef __SAM3X8E__ 
  PRINT_CSTSTR("%s","SAM3X8E ARM Cortex-M3 detected\n");
#endif

  // Power ON the module
  sx1272.ON();

#ifdef WITH_EEPROM
  // get config from EEPROM
  EEPROM.get(0, my_sx1272config);

  // found a valid config?
  if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x35) {
    PRINT_CSTSTR("%s","Get back previous sx1272 config\n");

    // set sequence number for SX1272 library
    sx1272._packetNumber=my_sx1272config.seq;
    PRINT_CSTSTR("%s","Using packet sequence number of ");
    PRINT_VALUE("%d", sx1272._packetNumber);
    PRINTLN;

#ifdef FORCE_DEFAULT_VALUE
    PRINT_CSTSTR("%s","Forced to use default parameters\n");
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x35;
    my_sx1272config.seq=sx1272._packetNumber;
    my_sx1272config.addr=node_addr;
    my_sx1272config.idle_period=idlePeriodInMin;    
    my_sx1272config.overwrite=0;
    EEPROM.put(0, my_sx1272config);
#else
    // get back the node_addr
    if (my_sx1272config.addr!=0 && my_sx1272config.overwrite==1) {
      
        PRINT_CSTSTR("%s","Used stored address\n");
        node_addr=my_sx1272config.addr;        
    }
    else
        PRINT_CSTSTR("%s","Stored node addr is null\n"); 

    // get back the idle period
    if (my_sx1272config.idle_period!=0 && my_sx1272config.overwrite==1) {
      
        PRINT_CSTSTR("%s","Used stored idle period\n");
        idlePeriodInMin=my_sx1272config.idle_period;        
    }
    else
        PRINT_CSTSTR("%s","Stored idle period is null\n");                 
#endif  

#if defined WITH_AES && not defined EXTDEVADDR && not defined LORAWAN
    DevAddr[3] = (unsigned char)node_addr;
#endif            
    PRINT_CSTSTR("%s","Using node addr of ");
    PRINT_VALUE("%d", node_addr);
    PRINTLN;   

    PRINT_CSTSTR("%s","Using idle period of ");
    PRINT_VALUE("%d", idlePeriodInMin);
    PRINTLN;     
  }
  else {
    // otherwise, write config and start over
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x35;
    my_sx1272config.seq=sx1272._packetNumber;
    my_sx1272config.addr=node_addr;
    my_sx1272config.idle_period=idlePeriodInMin;
    my_sx1272config.overwrite=0;
  }
#endif
  
#ifdef LORAWAN
  local_lorawan_init(SF);
#else

  // Set transmission mode and print the result
  e = sx1272.setMode(loraMode);
  PRINT_CSTSTR("%s","Setting Mode: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
#ifdef MY_FREQUENCY
  e = sx1272.setChannel(MY_FREQUENCY*1000000.0*RH_LORA_FCONVERT);
  PRINT_CSTSTR("%s","Setting customized frequency: ");
  PRINT_VALUE("%f", MY_FREQUENCY);
  PRINTLN;
#else
  e = sx1272.setChannel(DEFAULT_CHANNEL);  
#endif  
  PRINT_CSTSTR("%s","Setting frequency: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
#endif

  // enable carrier sense
  sx1272._enableCarrierSense=true;  
#ifdef LOW_POWER
  // TODO: with low power, when setting the radio module in sleep mode
  // there seem to be some issue with RSSI reading
  sx1272._RSSIonSend=false;
#endif  

#ifdef USE_LORAWAN_SW
  e = sx1272.setSyncWord(0x34);
  PRINT_CSTSTR("%s","Set sync word to 0x34: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
#endif
  
  // Select amplifier line; PABOOST or RFO
#ifdef PABOOST
  sx1272._needPABOOST=true;
#endif

  e = sx1272.setPowerDBM((uint8_t)MAX_DBM);

  //e = sx1272.setPowerDBM((uint8_t)20);
  
  PRINT_CSTSTR("%s","Setting Power: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(node_addr);
  PRINT_CSTSTR("%s","Setting node addr: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;

#ifdef MY_LORA_PREAMBLE_LENGTH
  sx1272.setPreambleLength(MY_LORA_PREAMBLE_LENGTH);
  e = sx1272.getPreambleLength();
  PRINT_CSTSTR("%s","Get Preamble Length: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  PRINT_CSTSTR("%s","Preamble Length: ");
  PRINT_VALUE("%d", sx1272._preamblelength);
  PRINTLN;
#endif
      
#ifdef WITH_LSC
  //need to initialize the LSC encoder
  //each time you want to change the session key, you need to call this function  
  LSC_session_init();
#endif
  
  // Print a success message
  PRINT_CSTSTR("%s","SX1272 successfully configured\n");

  //printf_begin();
  delay(500);
}


/*****************************
 _                       
| |                      
| |     ___   ___  _ __  
| |    / _ \ / _ \| '_ \ 
| |___| (_) | (_) | |_) |
\_____/\___/ \___/| .__/ 
                  | |    
                  |_|    
*****************************/

void loop(void)
{
  long startSend;
  long endSend;
  uint8_t app_key_offset=0;
  int e;
  float temp;
  
#ifndef LOW_POWER
  // 600000+random(15,60)*1000
  if (millis() > nextTransmissionTime) {
#else
      //time for next wake up
      nextTransmissionTime=millis()+(unsigned long)idlePeriodInMin*60*1000;
#endif      

#ifdef LOW_POWER
      digitalWrite(PIN_POWER,HIGH);
      // security?
      delay(200);    
#endif

      temp = 0.0;
      
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // change here how the temperature should be computed depending on your sensor type
      //  
    
      for (int i=0; i<5; i++) {
          temp += sensor_getValue();  
          delay(100);
      }

      //
      // 
      // /////////////////////////////////////////////////////////////////////////////////////////////////////////// 
      
#ifdef LOW_POWER
      digitalWrite(PIN_POWER,LOW);
#endif
      
      PRINT_CSTSTR("%s","Mean temp is ");
      temp = temp/5;
      PRINT_VALUE("%f", temp);
      PRINTLN;

      //for testing
      //temp = 22.5;
      
#if defined WITH_APPKEY && not defined LORAWAN
      app_key_offset = sizeof(my_appKey);
      // set the app key in the payload
      memcpy(message,my_appKey,app_key_offset);
#endif

      uint8_t r_size;

      // the recommended format if now \!TC/22.5
#ifdef STRING_LIB
      //r_size=sprintf((char*)message+app_key_offset,"\\!%s/%s",nomenclature_str,String(temp).c_str());
      
      //for range test with a LoRaWAN gateway on TTN
      //r_size=sprintf((char*)message+app_key_offset,"Hello from UPPA");
      r_size=sprintf((char*)message+app_key_offset,"Tks UPPA");
      
#else
      char float_str[10];
      ftoa(float_str,temp,2);
      // this is for testing, uncomment if you just want to test, without a real temp sensor plugged
      //strcpy(float_str, "21.55567");
      r_size=sprintf((char*)message+app_key_offset,"\\!%s/%s",nomenclature_str,float_str);
#endif

      PRINT_CSTSTR("%s","Sending ");
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      
      PRINT_CSTSTR("%s","Real payload size is ");
      PRINT_VALUE("%d", r_size);
      PRINTLN;
      
      int pl=r_size+app_key_offset;

      uint8_t p_type=PKT_TYPE_DATA;
      
#if defined WITH_AES || defined WITH_LSC
      // indicate that payload is encrypted
      p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;
#endif

#ifdef WITH_APPKEY
      // indicate that we have an appkey
      p_type = p_type | PKT_FLAG_DATA_WAPPKEY;
#endif

      sx1272.setPacketType(p_type);      
      
/**********************************  
  ___   _____ _____ 
 / _ \ |  ___/  ___|
/ /_\ \| |__ \ `--. 
|  _  ||  __| `--. \
| | | || |___/\__/ /
\_| |_/\____/\____/ 
***********************************/
//use AES (LoRaWAN-like) encrypting
///////////////////////////////////
#ifdef WITH_AES
#ifdef LORAWAN
      pl=local_aes_lorawan_create_pkt(message, pl, app_key_offset, true);
#else
      pl=local_aes_lorawan_create_pkt(message, pl, app_key_offset, false);
#endif
#endif

/********************************** 
 _      _____ _____ 
| |    /  ___/  __ \
| |    \ `--.| /  \/
| |     `--. \ |    
| |____/\__/ / \__/\
\_____/\____/ \____/
***********************************/
//use our Lightweight Stream Cipher (LSC) encrypting
////////////////////////////////////////////////////
#ifdef WITH_LSC
      pl=local_lsc_create_pkt(message, pl, app_key_offset, p_type, node_addr);
#endif
      
      //sx1272.CarrierSense();

      startSend=millis();
      
      // Send message to the gateway and print the result
      // with the app key if this feature is enabled
#ifdef WITH_ACK
      int n_retry=NB_RETRIES;
      
      do {
        e = sx1272.sendPacketTimeoutACK(DEFAULT_DEST_ADDR, message, pl);

        if (e==3)
          PRINT_CSTSTR("%s","No ACK");
        
        n_retry--;
        
        if (n_retry)
          PRINT_CSTSTR("%s","Retry");
        else
          PRINT_CSTSTR("%s","Abort"); 
          
      } while (e && n_retry);          
#else
      e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, pl);
#endif
  
      endSend=millis();

#ifdef LORAWAN
      // switch back to normal behavior
      sx1272._rawFormat=false;
#endif
          
#ifdef WITH_EEPROM
      // save packet number for next packet in case of reboot
      my_sx1272config.seq=sx1272._packetNumber;
      EEPROM.put(0, my_sx1272config);
#endif

      PRINT_CSTSTR("%s","LoRa pkt size ");
      PRINT_VALUE("%d", pl);
      PRINTLN;
      
      PRINT_CSTSTR("%s","LoRa pkt seq ");
      PRINT_VALUE("%d", sx1272.packet_sent.packnum);
      PRINTLN;
    
      PRINT_CSTSTR("%s","LoRa Sent in ");
      PRINT_VALUE("%ld", endSend-startSend);
      PRINTLN;
          
      PRINT_CSTSTR("%s","LoRa Sent w/CAD in ");
      PRINT_VALUE("%ld", endSend-sx1272._startDoCad);
      PRINTLN;
       
      PRINT_CSTSTR("%s","Packet sent, state ");
      PRINT_VALUE("%d", e);
      PRINTLN;

#ifdef WITH_RCVW
      PRINT_CSTSTR("%s","Wait for ");
      PRINT_VALUE("%d", DELAY_BEFORE_RCVW-1000);
      PRINTLN;
      //wait a bit
      delay(DELAY_BEFORE_RCVW-1000);

      PRINT_CSTSTR("%s","Wait for incoming packet\n");
      // wait for incoming packets
      e = sx1272.receivePacketTimeout(10000);

      // we have received a downlink message
      //
      if (!e) {
         int i=0;
         int cmdValue;
         uint8_t tmp_length;

         sx1272.getSNR();
         sx1272.getRSSIpacket();
         
         tmp_length=sx1272._payloadlength;

         sprintf((char*)message, "^p%d,%d,%d,%d,%d,%d,%d\n",
                   sx1272.packet_received.dst,
                   sx1272.packet_received.type,                   
                   sx1272.packet_received.src,
                   sx1272.packet_received.packnum, 
                   tmp_length,
                   sx1272._SNR,
                   sx1272._RSSIpacket);
                                   
         PRINT_STR("%s",(char*)message);         
         
         for ( ; i<tmp_length; i++) {
           PRINT_STR("%c",(char)sx1272.packet_received.data[i]);
           
           message[i]=(char)sx1272.packet_received.data[i];
         }
         
         message[i]=(char)'\0';    
         PRINTLN;
         FLUSHOUTPUT;   

        i=0;

        // commands have following format /@A6#
        //
        if (message[i]=='/' && message[i+1]=='@') {
    
            PRINT_CSTSTR("%s","Parsing command\n");      
            i=i+2;   

            switch ((char)message[i]) {

#ifndef LORAWAN
                  // set the node's address, /@A10# to set the address to 10 for instance
                  case 'A': 

                      i++;
                      cmdValue=getCmdValue(i);
                      
                      // cannot set addr greater than 255
                      if (cmdValue > 255)
                              cmdValue = 255;
                      // cannot set addr lower than 2 since 0 is broadcast and 1 is for gateway
                      if (cmdValue < 2)
                              cmdValue = node_addr;
                      // set node addr        
                      node_addr=cmdValue; 
#ifdef WITH_AES 
                      DevAddr[3] = (unsigned char)node_addr;
#endif
                      
                      PRINT_CSTSTR("%s","Set LoRa node addr to ");
                      PRINT_VALUE("%d", node_addr);  
                      PRINTLN;
                      // Set the node address and print the result
                      e = sx1272.setNodeAddress(node_addr);
                      PRINT_CSTSTR("%s","Setting LoRa node addr: state ");
                      PRINT_VALUE("%d",e);     
                      PRINTLN;           

#ifdef WITH_EEPROM
                      // save new node_addr in case of reboot
                      my_sx1272config.addr=node_addr;
                      my_sx1272config.overwrite=1;
                      EEPROM.put(0, my_sx1272config);
#endif
                      break;        
#endif
                  // set the time between 2 transmissions, /@I10# to set to 10 minutes for instance
                  case 'I': 

                      i++;
                      cmdValue=getCmdValue(i);

                      // cannot set addr lower than 1 minute
                      if (cmdValue < 1)
                              cmdValue = idlePeriodInMin;
                      // idlePeriodInMin      
                      idlePeriodInMin=cmdValue; 
                      
                      PRINT_CSTSTR("%s","Set duty-cycle to ");
                      PRINT_VALUE("%d", idlePeriodInMin);  
                      PRINTLN;         

#ifdef WITH_EEPROM
                      // save new node_addr in case of reboot
                      my_sx1272config.idle_period=idlePeriodInMin;
                      my_sx1272config.overwrite=1;
                      EEPROM.put(0, my_sx1272config);
#endif

                      break;  

                  // Toggle a LED to illustrate an actuation example
                  // command syntax is /@L2# for instance
                  case 'L': 

                      i++;
                      cmdValue=getCmdValue(i);
                      
                      PRINT_CSTSTR("%s","Toggle LED on pin ");
                      PRINT_VALUE("%d", cmdValue);
                      PRINTLN;

                      // warning, there is no check on the pin number
                      // /@L2# for instance will toggle LED connected to digital pin number 2
                      pinMode(cmdValue, OUTPUT);
                      digitalWrite(cmdValue, HIGH);
                      delay(500);
                      digitalWrite(cmdValue, LOW);
                      delay(500);
                      digitalWrite(cmdValue, HIGH);
                      delay(500);
                      digitalWrite(cmdValue, LOW);
                      
                      break;
                            
                  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                  // add here new commands
                  //  

                  //
                  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

                  default:
      
                    PRINT_CSTSTR("%s","Unrecognized cmd\n");       
                    break;
            }
        }          
      }
      else
        PRINT_CSTSTR("%s","No packet\n");
#endif

#if defined LOW_POWER && not defined _VARIANT_ARDUINO_DUE_X_
      PRINT_CSTSTR("%s","Switch to power saving mode\n");

      e = sx1272.setSleepMode();

      if (!e)
        PRINT_CSTSTR("%s","Successfully switch LoRa module in sleep mode\n");
      else  
        PRINT_CSTSTR("%s","Could not switch LoRa module in sleep mode\n");
        
      FLUSHOUTPUT
      delay(10);

      //how much do we still have to wait, in millisec?
      unsigned long now_millis=millis();
      unsigned long waiting_t = nextTransmissionTime-now_millis;
      
#ifdef __SAMD21G18A__
      // For Arduino M0 or Zero we use the built-in RTC
      rtc.setTime(17, 0, 0);
      rtc.setDate(1, 1, 2000);
      rtc.setAlarmTime(17, idlePeriodInMin, 0);
      // for testing with 20s
      //rtc.setAlarmTime(17, 0, 20);
      rtc.enableAlarm(rtc.MATCH_HHMMSS);
      //rtc.attachInterrupt(alarmMatch);
      rtc.standbyMode();
      
      LowPower.standby();

      PRINT_CSTSTR("%s","SAMD21G18A wakes up from standby\n");      
      FLUSHOUTPUT
#else

#if defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
      // warning, setTimer accepts value from 1ms to 65535ms max
      // milliseconds
      // by default, LOW_POWER_PERIOD is 60s for those microcontrollers
      timer.setTimer(LOW_POWER_PERIOD*1000);
#endif

      //nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
      
      while (waiting_t>0) {  

#if defined ARDUINO_AVR_MEGA2560 || defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MINI || defined __AVR_ATmega32U4__    
          // ATmega2560, ATmega328P, ATmega168, ATmega32U4
          // each wake-up introduces an overhead of about 158ms
          if (waiting_t > 8158) {
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 8158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","8");
#endif              
          }
          else if (waiting_t > 4158) {
            LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 4158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","4");
#endif 
          }
          else if (waiting_t > 2158) {
            LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 2158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","2");
#endif 
          }
          else if (waiting_t > 1158) {
            LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
            waiting_t = waiting_t - 1158;
#ifdef SHOW_LOW_POWER_CYCLE                  
                  PRINT_CSTSTR("%s","1");
#endif 
          }      
          else {
            delay(waiting_t); 
#ifdef SHOW_LOW_POWER_CYCLE                   
                  PRINT_CSTSTR("%s","D[");
                  PRINT_VALUE("%d", waiting_t);
                  PRINT_CSTSTR("%s","]");
#endif
            waiting_t = 0;
          }

#ifdef SHOW_LOW_POWER_CYCLE
          FLUSHOUTPUT
          delay(1);
#endif
          
#elif defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
          // Teensy31/32 & TeensyLC
          if (waiting_t < LOW_POWER_PERIOD*1000) {
            timer.setTimer(waiting_t);
            waiting_t = 0;
          }
          else
            waiting_t = waiting_t - LOW_POWER_PERIOD*1000;
                        
#ifdef LOW_POWER_HIBERNATE
          Snooze.hibernate(sleep_config);
#else            
          Snooze.deepSleep(sleep_config);
#endif

#else
          // use the delay function
          delay(waiting_t);
          waiting_t = 0;
#endif                                            
       }

      /*
      for (uint8_t i=0; i<nCycle; i++) {  

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MINI || defined __AVR_ATmega32U4__         
          // ATmega328P, ATmega168, ATmega32U4
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
          
          //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
          //              SPI_OFF, USART0_OFF, TWI_OFF);
#elif defined ARDUINO_AVR_MEGA2560
          // ATmega2560
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
          
          //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, 
          //      TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, 
          //      USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);
#elif defined __MK20DX256__ || defined __MKL26Z64__ || defined __MK64FX512__ || defined __MK66FX1M0__
          // Teensy31/32 & TeensyLC
#ifdef LOW_POWER_HIBERNATE
          Snooze.hibernate(sleep_config);
#else            
          Snooze.deepSleep(sleep_config);
#endif  
#else
          // use the delay function
          delay(LOW_POWER_PERIOD*1000);
#endif                        
          PRINT_CSTSTR("%s",".");
          FLUSHOUTPUT
          delay(10);                        
      }
      */
#endif  
      
#else
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
      PRINT_CSTSTR("%s","Will send next value at\n");
      // can use a random part also to avoid collision
//      nextTransmissionTime=millis()+(unsigned long)idlePeriodInMin*60*1000; //+(unsigned long)random(15,60)*1000;
nextTransmissionTime=millis()+15000; //+(unsigned long)random(15,60)*1000;
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
  }
#endif
}
