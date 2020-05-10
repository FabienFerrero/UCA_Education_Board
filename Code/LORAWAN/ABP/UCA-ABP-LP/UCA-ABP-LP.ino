/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This uses ABP (Activation-by-personalisation), where a DevAddr and
 * Session keys are preconfigured (unlike OTAA, where a DevEUI and
 * application key is configured, while the DevAddr and session keys are
 * assigned/generated in the over-the-air-activation procedure).
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

/*******************************************************************************/
 // Region definition (will change de frequency bands
 // Define only 1 country
 //
#define CFG_EU 1
//#define CFG_VN 1

/*******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "LowPower.h"
//Sensors librairies


#define debugSerial Serial
//#define SHOW_DEBUGINFO
#define debugPrintLn(...) { if (debugSerial) debugSerial.println(__VA_ARGS__); }
#define debugPrint(...) { if (debugSerial) debugSerial.print(__VA_ARGS__); }




//Define sensor PIN
#define LAPIN A3 // PIN with Light sensor analog output 
#define LPPIN 5 // PIN with Light power input

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x2601162F;

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const PROGMEM u1_t NWKSKEY[16] = { 0xD7, 0x6C, 0x09, 0xE9, 0xD4, 0x80, 0x45, 0xFC, 0xB7, 0x92, 0x70, 0x17, 0x39, 0x85, 0x1F, 0x02 };


// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const u1_t PROGMEM APPSKEY[16] = { 0x85, 0x43, 0xEB, 0x8C, 0x13, 0xC9, 0x7A, 0xB8, 0x8A, 0x57, 0x77, 0xEE, 0xFD, 0x57, 0x05, 0xAE };

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;



// global enviromental parameters (exemples)
static float temp = 0.0;
static float pressure = 0.0;
static float humidity = 0.0;
static float batvalue;
static float light;



// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 8,
    .dio = {3, 7, 6},
};



// ---------------------------------------------------------------------------------
// Functions
// ---------------------------------------------------------------------------------

extern volatile unsigned long timer0_overflow_count;
extern volatile unsigned long timer0_millis;
void addMillis(unsigned long extra_millis) {
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis += extra_millis;
  SREG = oldSREG;
  sei();
}

void do_sleep(unsigned int sleepyTime) {
  unsigned int eights = sleepyTime / 8;
  unsigned int fours = (sleepyTime % 8) / 4;
  unsigned int twos = ((sleepyTime % 8) % 4) / 2;
  unsigned int ones = ((sleepyTime % 8) % 4) % 2;

        #ifdef SHOW_DEBUGINFO
          debugPrint(F("Sleeping for "));
          debugPrint(sleepyTime);
          debugPrint(F(" seconds = "));
          debugPrint(eights);
          debugPrint(F(" x 8 + "));
          debugPrint(fours);
          debugPrint(F(" x 4 + "));
          debugPrint(twos);
          debugPrint(F(" x 2 + "));
          debugPrintLn(ones);
          delay(500); //Wait for serial to complete
        #endif
        
        
          for ( int x = 0; x < eights; x++) {
            // put the processor to sleep for 8 seconds
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            // LMIC uses micros() to keep track of the duty cycle, so
              // hack timer0_overflow for a rude adjustment:
              cli();
              timer0_overflow_count+= 8 * 64 * clockCyclesPerMicrosecond();
              sei();
          }
          for ( int x = 0; x < fours; x++) {
            // put the processor to sleep for 4 seconds
            LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
            // LMIC uses micros() to keep track of the duty cycle, so
              // hack timer0_overflow for a rude adjustment:
              cli();
              timer0_overflow_count+= 4 * 64 * clockCyclesPerMicrosecond();
              sei();
          }
          for ( int x = 0; x < twos; x++) {
            // put the processor to sleep for 2 seconds
            LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
            // LMIC uses micros() to keep track of the duty cycle, so
              // hack timer0_overflow for a rude adjustment:
              cli();
              timer0_overflow_count+= 2 * 64 * clockCyclesPerMicrosecond();
              sei();
          }
          for ( int x = 0; x < ones; x++) {
            // put the processor to sleep for 1 seconds
            LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
            // LMIC uses micros() to keep track of the duty cycle, so
              // hack timer0_overflow for a rude adjustment:
              cli();
              timer0_overflow_count+=  64 * clockCyclesPerMicrosecond();
              sei();
          }
          addMillis(sleepyTime * 1000);
}

// ReadVcc function to read MCU Voltage
long readVcc() {
      long result;
      // Read 1.1V reference against AVcc
      ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
      delay(2); // Wait for Vref to settle
      ADCSRA |= _BV(ADSC); // Convert
      while (bit_is_set(ADCSRA,ADSC));
      result = ADCL;
      result |= ADCH<<8;
      result = 1126400L / result; // Back-calculate AVcc in mV
      return result;
}

// Read Light function for TEMT6000
float readLight() { 
        float result;
        // Light sensor Voltage
      digitalWrite(LPPIN, HIGH); // Power the sensor
      delay(1);
      int sensorValue = analogRead(LAPIN);
        // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
        float voltage = sensorValue * (batvalue / 1023.0)/100; // Batvalue is in tens of mV, so the result has to be divided by 100
      result = voltage*200; // multiply by 2000 to have Lx
      digitalWrite(LPPIN, LOW); // switch off the sensor
        return result;
}

// Function to update sensor values

void updateEnvParameters()
{
   
       temp = 20.0;
       humidity = 50;
       light = readLight();
       batvalue = (int)(readVcc()/10);  // readVCC returns in tens of mVolt 
          
      
        #ifdef SHOW_DEBUGINFO
        // print out the value you read:
        Serial.print("Humidity : ");
        Serial.println(humidity);
        Serial.print("T°c : ");
        Serial.println(temp);
        Serial.print("Vbatt : ");
        Serial.println(batvalue);
        #endif
  
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
              for (int i = 0; i < LMIC.dataLen; i++) {
              if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
              Serial.print(F("0"));
              }
              Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
              }
              Serial.println("");
            }
            // Schedule next transmission
            Serial.end();
            os_setTimedCallback(&sendjob,os_getTime()+sec2osticks(1), do_send);
            do_sleep(TX_INTERVAL);
            Serial.begin(115200);
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } 
    
    else {      

         updateEnvParameters();
               
        
        #ifdef SHOW_DEBUGINFO
            debugPrint(F("T="));
            debugPrintLn(temp);
        
            debugPrint(F("H="));
            debugPrintLn(humidity);
            debugPrint(F("L="));
            debugPrintLn(light);
            debugPrint(F("BV="));
            debugPrintLn(batvalue);
        #endif
            int t = (int)((temp) / 10.0);
            int h = (int)(humidity / 50.0);
            int bat = batvalue; // multifly by 10 for V in Cayenne
            int l = light; // light sensor in Lx
        
            unsigned char mydata[15];
            mydata[0] = 0x1; // CH1
            mydata[1] = 0x67; // Temp
            mydata[2] = t >> 8;
            mydata[3] = t & 0xFF;
            mydata[4] = 0x2; // CH2
            mydata[5] = 0x68; // Humidity
            mydata[6] = h & 0xFF;
            mydata[7] = 0x3; // CH3
            mydata[8] = 0x2; // Analog output
            mydata[9] = bat >> 8;
            mydata[10] = bat & 0xFF;
            mydata[11] = 0x4; // CH4
            mydata[12] = 0x65; // Luminosity
            mydata[13] = l >> 8;
            mydata[14] = l & 0xFF;
            
            LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
            #ifdef SHOW_DEBUGINFO
            debugPrintLn(F("PQ")); //Packet queued
            Serial.println(F("Packet queued"));
            #endif

        
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {

    Serial.begin(115200);
    Serial.println(F("Starting"));
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    #ifdef SHOW_DEBUGINFO
    debugPrintLn(F("Starting"));
    delay(100);
    #endif  

   updateEnvParameters(); // To have value for the first Tx
  

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();


    /* This function is intended to compensate for clock inaccuracy (up to ±10% in this example), 
    but that also works to compensate for inaccuracies due to software delays. 
    The downside of this compensation is a longer receive window, which means a higher battery drain. 
    So if this helps, you might want to try to lower the percentage (i.e. lower the 10 in the above call), 
    often 1% works well already. */
    
    LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    #if defined(CFG_EU)
    // Set up the 8 channels used    
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    
    #elif defined(CFG_VN)
    // Set up the 8 channels used    
    LMIC_setupChannel(0, 921400000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 921600000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 921800000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 922000000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 922200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 922400000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 922600000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 922800000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 922700000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band    
    #endif

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop() {
  
    os_runloop_once();
}
