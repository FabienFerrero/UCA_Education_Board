#include <Wire.h>
#include <SI7021.h>


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


SI7021 sensor;



void temp2led(int temp){

  
  int hue = 360 -(temp*40)+700; // Formmula to get yellow at 22°

  long color = HSBtoRGB(hue, saturation, brightness);
 
 // Get the red, blue and green parts from generated color
 int red = color >> 16 & 255;
 int green = color >> 8 & 255;
 int blue = color & 255;
 setColor(red, blue, green); 
} 
  
 void setColor(int redValue,  int blueValue, int greenValue) {
fill_solid( leds, NUM_LEDS, CRGB(greenValue,redValue,blueValue));
FastLED.show();

}

void setup() {
    Serial.begin(115200);
    sensor.begin();
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    
  
}

void loop() {

    // temperature is an integer in hundredths
    float temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100;
    Serial.print("Temperature :");  
    Serial.print(temperature);
    Serial.println("°Celcius");

temp2led(temperature);
    
    delay(500);
    
    // humidity is an integer representing percent
    int humidity = sensor.getHumidityPercent();
    Serial.print("Humidity :");  
    Serial.print(humidity);
    Serial.println("%");
    
    
    delay(500);
    
    
    // enable internal heater for testing
    sensor.setHeater(true);
      Serial.println("Activate heater");  


    for( int i=0; i<=20; i++ ) {
    temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100;
    temp2led(temperature);
    Serial.print("Temperature :");  
    Serial.print(temperature);
    Serial.println("°Celcius");
    delay (1000);
      }
       
    
    sensor.setHeater(false);
    Serial.println("Desactivate heater");
    
    // see if heater changed temperature
    temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100;
    temp2led(temperature);
    Serial.print("Temperature :");  
    Serial.print(temperature);
    Serial.println("°Celcius");
    
    
    //cool down
    for( int i=0; i<=20; i++ ) {
    temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100;
    temp2led(temperature);
    Serial.print("Temperature :");  
    Serial.print(temperature);
    Serial.println("°Celcius");
    delay (1000);
      }
    

    // get humidity and temperature in one shot, saves power because sensor takes temperature when doing humidity anyway
    si7021_env data = sensor.getHumidityAndTemperature();
    Serial.print("Temperature :");  
    Serial.print(data.celsiusHundredths/100);
    Serial.println("°Celcius");
    
    delay(500);
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
