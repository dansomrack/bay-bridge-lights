#include "FastLED.h"

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    832
#define NUM_STRANDS  36

#define MAX_VOLTAGE  3
#define MAX_MILLIAMP  4000

CRGB leds[NUM_LEDS];
int strandLength[NUM_STRANDS] = {2,5,8,14,20,26,35,44,54,  54,44,35,26,20,14,8,5,2,    2,5,8,14,20,26,35,44,54, 54,44,35,26,20,14,8,5,2};
//int strandLength[NUM_STRANDS] = {2,5,8,14,20,26,35,44,54};
int ledStart[NUM_STRANDS];

#define BRIGHTNESS          90
#define FRAMES_PER_SECOND  30

// RAIN PARAMS
int rainVelocity[NUM_STRANDS] = {10,20,15,12,30,25,28,5,27, 19,13,15,17,29,25,29,5,27, 11,19,16,13,30,25,22,24,28, 9,17,19,10,28,25,23,7,20};
//int rainVelocity[NUM_STRANDS] = {29,31,30,31,30,29,30,29,28};
// END RAIN PARAMS


void setup() {
  Serial.begin(57600);
  Serial.println("resetting");
  
  delay(3000); // 3 second delay for recovery
  
  //Inititalize the ledStartArray
  initializeLedStart(strandLength);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  //Set the maximum milliamps and voltage
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTAGE,MAX_MILLIAMP);  

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  //Clear the strand
  fill_solid(leds, NUM_LEDS, CRGB::Black); 
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rain3, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { rain2};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int iterator = 0; // rotating "base color" used by many of the patterns
  
void loop()
{  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();
  //rain(iterator, 3);
  //rain3();
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  
  iterator++;
  
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rain2()
{
  uint16_t i = iterator;
  int ledAddress;
  for( int x = 0; x < NUM_STRANDS; x++) {
    int y = rainVelocity[x]+i % strandLength[x];
    
    ledAddress = XY(x,y);
    if (ledAddress != -1)
    {
      leds[ledAddress] = CHSV(0, 255, 255);
    }
    for(int i = 0; i < NUM_LEDS; i++) 
    { 
      leds[i].nscale8(250); 
    } 

  }
}

void rain3()
{
 //  sub pixel level defines how many ghost steps exist between pixels and allows
  //  the rain effect to advance at various rates per column
  uint8_t subPixelLevel = 10;
  uint8_t tallestCol = 54;
  int iMax = subPixelLevel * tallestCol;
  uint16_t i = iterator;
  uint16_t tailLength = 4;
  uint8_t value = 255;
  int ledAddress;

  for( int x = 0; x < NUM_STRANDS; x++) {
    int y = rainVelocity[x]*i % iMax;
    y = map(y, 0, iMax, 0, strandLength[x]);
    //y = map(y, 0, iMax, 0, tallestCol);
    
   int reversey =  strandLength[x] - y;
    ledAddress = XY(x,reversey);    
    if (ledAddress != -1)
    {
      leds[ledAddress] = CRGB::White;
    }   
  }
    fadeall();
}

void rain(int i, uint8_t tailLength)
{
  //  sub pixel level defines how many ghost steps exist between pixels and allows
  //  the rain effect to advance at various rates per column
  uint8_t subPixelLevel = 10;
  uint8_t tallestCol = 54;
  int iMax = tallestCol * subPixelLevel;
  uint8_t value = 255;
  int ledAddress;
  
  blankOut();
  for( int x = 0; x < NUM_STRANDS; x++) {
    int y = rainVelocity[x]*i % iMax;
    y = map(y, 0, iMax, 0, tallestCol);
    
    ledAddress = XY(x,y);
    if (ledAddress != -1)
    {
      leds[ledAddress] = CHSV(0, 255, 255);
    }
    for(uint8_t tailIndex=0; tailIndex<tailLength; tailIndex++){
     ledAddress = XY(x,(y-tailIndex) % tallestCol);
     if (ledAddress != -1)
     {
      value = tailLength-tailIndex;
      leds[ledAddress] = CHSV(0, 255, value);
     }
    }
  }
}


void blankOut() 
{ 
// sets all LEDs to off
  for(int i = 0; i < NUM_LEDS; i++) 
  { 
    leds[i] = CHSV(0, 0, 0);
  } 
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(500,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


uint16_t XY( uint8_t x, uint8_t y)
{
    uint16_t i;
	
    //if even row
    if( x % 2 == 0) {
      // even rows run down
      Serial.println("Even row");
      uint8_t reverseY = (strandLength[x]-1) - y;
      Serial.print("reverseY: ");
      Serial.println(reverseY, DEC);
      i = ledStart[x] + reverseY;
    }
    else {
      // Even rows run forwards
      i = ledStart[x] + y;
    }
    
    if( y > strandLength[x] ){
      return -1;
    } else {
      return i;
    }
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
    leds[i].nscale8(150); 
  } 
}
