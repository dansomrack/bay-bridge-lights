#include "FastLED.h"

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    208
#define NUM_STRANDS  9

#define MAX_VOLTAGE  3
#define MAX_MILLIAMP  500

CRGB leds[NUM_LEDS];
int strandLength[NUM_STRANDS] = {54,44,35,26,20,14,8,5,2};
int ledStart[NUM_STRANDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120


void setup() {
  Serial.begin(57600);
  Serial.println("resetting");
  
  delay(3000); // 3 second delay for recovery
  
  //Inititalize the ledStartArray
  initializeLedStart(strandLength);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  //Set the maximum milliamps and voltage
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTAGE,MAX_MILLIAMP);  
  
  //Clear the strand
  fill_solid(leds, NUM_LEDS, CRGB::Black); 
}

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t Hue = 0; // rotating "base color" used by many of the patterns

void loop()
{
  static uint8_t hue = 0;
  for( int x = 0; x < NUM_STRANDS; x++) {
    for( int y = 0; y < strandLength[x]; y++) {
      Serial.print("x: ");
      Serial.println(x, DEC);
      Serial.print("y: ");
      Serial.println(y, DEC);
      
      int index = XY(x,y);
      leds[index] = CHSV(hue++, 255, 255);
      
      Serial.print("XY: ");
      Serial.println(index, DEC);
      
      // Show the leds
      FastLED.show(); 

      fadeall();
      
      // Wait a little bit before we loop around and do it again
      delay(50); 
    }
  }

}


uint16_t XY( uint8_t x, uint8_t y)
{
    uint16_t i;
	
    //if odd row
    if( x % 2 > 0) {
      // Odd rows run backwards
      Serial.println("Odd row");
      uint8_t reverseY = (strandLength[x]-1) - y;
      Serial.print("reverseY: ");
      Serial.println(reverseY, DEC);
      i = ledStart[x] + reverseY;
    }
    else {
      // Even rows run forwards
      i = ledStart[x] + y;
    }
    return i;
}

//This fills in the LEDStart array based on the strand lengths 
void initializeLedStart(int *strandLengthIn)
{
  int runningTotal = 0;
  for(int i = 0; i<NUM_STRANDS; i++)
  {
    ledStart[i] = runningTotal;
    runningTotal += strandLengthIn[i];   
    Serial.print("Led Start: "); 
    Serial.println(ledStart[i],DEC);  
    Serial.print("Strand length: "); 
    Serial.println(strandLengthIn[i],DEC); 
  }  
}

void fadeall() 
{ 
  for(int i = 0; i < NUM_LEDS; i++) 
  { 
    leds[i].nscale8(50); 
  } 
}
