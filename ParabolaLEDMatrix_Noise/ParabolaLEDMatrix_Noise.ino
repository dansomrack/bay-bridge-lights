/* Required Libraries:
 *  FastLED //https://github.com/FastLED/FastLED
 *  FastLED_GFX
 */

#include <FastLED.h>        //https://github.com/FastLED/FastLED
#include <FastLED_GFX.h>    
#include <Fonts/FreeMono12pt7b.h>
#include "LEDParabolaMatrix.h"
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

//Power and Hardware Configuration
#define MAX_VOLTAGE  3
#define MAX_MILLIAMP  4000
#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

//LED Strip Configuration 
#define NUM_LEDS    828
#define NUM_STRANDS  36
#define MATRIX_WIDTH   36 // width of matrix
#define MATRIX_HEIGHT  53 // height of matrix
#define MAX_DIMENSION 53
//int16_t strandLength[NUM_STRANDS] = {54,43,35,26,20,14,8,5,2}; 2,5,8,14,20,26,35,43,54,  54,43,35,26,20,14,8,5,2,   
int16_t strandLength[NUM_STRANDS] = { 2,5,8,14,20,26,35,44,53, 53,44,35,26,20,14,8,5,2, 2,5,8,14,20,26,35,44,53, 53,44,35,26,20,14,8,5,2};

//LED Datastrutures
LEDParabolaMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, strandLength, NUM_LEDS> leds;

//Effect Constants
//#define BRIGHTNESS          90
#define FRAMES_PER_SECOND  120

//Effect Variables
uint8_t brightness = 102;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int iterator = 0; 
uint8_t hue = 0;
int16_t counter = 0;

int CursorX = leds.Width();
const unsigned char TxtDemo[] = "Hello world";

// The 32bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;
// RAIN PARAMS
int rainVelocity[NUM_STRANDS] = {10,20,15,12,30,25,28,5,27, 19,13,15,17,29,25,29,5,27, 11,19,16,13,30,25,22,24,28, 9,17,19,10,28,25,23,7,20};
//int rainVelocity[NUM_STRANDS] = {29,31,30,31,30,29,30,29,28};
// END RAIN PARAMS


//bluetooth stuff
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];
uint8_t animationState = 1;
int pos = 0, dir = 1; // Position, direction of "eye" for larson scanner animation
 

void setup()
{
  //Initialize Fast LED Settings  
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTAGE,MAX_MILLIAMP);  
  FastLED.setBrightness(brightness);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds[0], leds.Size()).setCorrection(TypicalLEDStrip);
  FastLED.clear(true);
  delay(500);

  //Initialize Text Settings
  leds.setTextWrap(false);
  leds.setTextColor(CRGB::White);
  leds.setTextSize(4);

  //Initialize Noise Settings
  x = random16();
  y = random16();
  z = random16();

 Serial.begin(115200);
 setupBluetooth();

}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rain3, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fillStripes, noiseDisplay };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
//SimplePatternList gPatterns = { sinelon};

void loop()
{

  //FastLED.clear();   

  //fillStripes();
  // writeBasicText();
  // writeComplexText();
  //drawBlackHoc();
  //sinelon();
  //It is expected that each function handles its own clear
  gPatterns[gCurrentPatternNumber]();
   
  FastLED.show();

  // insert a delay to create the framerate
  FastLED.delay(1000/FRAMES_PER_SECOND);   

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 60 ) { nextPattern(); } // change patterns periodically
  EVERY_N_MILLISECONDS( 500 ){checkBluetooth();}
  
  iterator++;
  
}


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void previousPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber - 1) % ARRAY_SIZE( gPatterns);
}

//raise by 51 steps. 5 clicks for full brightness
void raiseBrightness()
{
  brightness = min(brightness+51, 255);  
  FastLED.setBrightness(brightness);
  Serial.print(F("Raising Brightness"));
}

void lowerBrightness()
{
  brightness = max(brightness-51, 0);  
  FastLED.setBrightness(brightness);
    Serial.print(F("Lowering Brightness"));
}

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE     0

void setupBluetooth()
{
    Serial.println(F("Adafruit Bluefruit Neopixel Color Picker Example"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
//  while (! ble.isConnected()) {
//      delay(500);
//  }

  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));
  
}

void checkBluetooth()
{
   /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  /* Got a packet! */
  printHex(packetbuffer, len);

  // Buttons
  if (packetbuffer[1] == 'B') { 
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    Serial.print ("Button "); Serial.print(buttnum);
    animationState = buttnum;
    if (pressed) {
      Serial.println(" pressed");
    } else {
      Serial.println(" released");
    }   
    
  } 

    if(animationState == 5)
  {
    raiseBrightness();
  }
  if(animationState == 6)
  {
    lowerBrightness();
  }

  if(animationState == 7)
  {
    previousPattern();
  }
  if(animationState == 8)
  {
    nextPattern();
  }
  


}


/* This file contains the effect library that gets cycled through*/
void fadeall() 
{ 
  for(int i = 0; i < NUM_LEDS; i++) 
  { 
    leds(i).nscale8(150); 
  } 
}

void blankOut() 
{ 
// sets all LEDs to off
  for(int i = 0; i < NUM_LEDS; i++) 
  { 
    leds(i) = CHSV(0, 0, 0);
  } 
}


void fillStripes()
{
  uint8_t h = hue, x, y;
  
  if (counter < 500)
  {
    // ** Fill LED's with diagonal stripes
    for (x=0; x<(leds.Width()+leds.Height()); ++x)
    {
      leds.DrawLine(x - leds.Height(), leds.Height() - 1, x, 0, CHSV(h, 255, 80));
      h+=16;
    }
  }
  else
  {
    // ** Fill LED's with horizontal stripes
    for (y=0; y<leds.Height(); ++y)
    {
      leds.DrawLine(0, y, leds.Width() - 1, y, CHSV(h, 255, 80));
      h+=16;
    }
  }
  hue+=4;

  
  counter++;
  if (counter >= 1000)
    counter = 0;

}

void writeBasicText()
{
  FastLED.clear(); 
  int16_t yLocation = 45;
  int16_t  textx1, texty1;
  uint16_t textw, texth;

  char char_array[] = "Hello World";
  
  leds.getTextBounds(char_array, 0, 0, &textx1, &texty1, &textw, &texth);
  
  leds.setCursor(CursorX, yLocation);
  //leds.setCursor(9, 40);
  leds.setFont(&FreeMono12pt7b);
  leds.print(char_array);
  CursorX--;
  Serial.print("textw");
  Serial.println(textw);
    Serial.println(textx1);
  if(CursorX < -1 * textw) {
    CursorX = leds.Width()-1;
    Serial.println("swapping");
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
    //y = map(y, 0, iMax, 0, strandLength[x]);
    y = map(y, 0, iMax, 0, tallestCol-1);
//    
//   int reversey =  strandLength[x] - y;
//    ledAddress = XY(x,reversey);    
//    if (ledAddress != -1)
//    {
//      leds[ledAddress] = CRGB::White;
//    }   
    
    leds(x,y) = CRGB::White;
    
  }
    fadeall();
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds.m_LED, NUM_LEDS, gHue, 7);
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
    
    leds( random16(NUM_LEDS) ) = CRGB::White;
  }
}

void confetti() 
{
//  // random colored speckles that blink in and fade smoothly
//  fadeToBlackBy( leds.m_LED, NUM_LEDS, 10);
//  int pos = random16(NUM_LEDS);
//  leds(pos) += CHSV( gHue + random8(64), 200, 255);
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds.m_LED, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds(beatsin16(i+7,0,NUM_LEDS)) = leds(beatsin16(i+7,0,NUM_LEDS)) | CHSV(dothue, 200, 255);
    dothue += 32;
  }

}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds.m_LED, NUM_LEDS, 60);
  int pos = beatsin16(10,0,leds.Width());
  //leds(pos) += CHSV( gHue, 255, 192);
  leds.DrawLine(pos, 0, pos, leds.Height()-1,  CHSV( gHue, 255, 192));
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds(i) = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds.m_LED, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    int pos = beatsin16(i+7,0,leds.Width());
    //leds(beatsin16(i+7,0,NUM_LEDS)) = leds(beatsin16(i+7,0,NUM_LEDS)) | CHSV(dothue, 200, 255);
    leds.DrawLine(pos, 0, pos, leds.Height()-1,  CHSV(dothue, 200, 255));
    dothue += 32;
  }
}


// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
// uint16_t speed = 1; // almost looks like a painting, moves very slowly
uint16_t speed = 10; // a nice starting speed, mixes well with a scale of 100
// uint16_t speed = 33;
// uint16_t speed = 100; // wicked fast!

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// uint16_t scale = 1; // mostly just solid colors
// uint16_t scale = 4011; // very zoomed out and shimmery
uint16_t scale = 50;

// This is the array that we keep our computed noise values in
uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += speed;
}


void noiseDisplay() {
  static uint8_t ihue=0;
  fillnoise8();
  for(int i = 0; i < MATRIX_WIDTH; i++) {
    for(int j = 0; j < MATRIX_HEIGHT; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.
     // leds.drawPixel(i,j, CHSV(noise[j][i],255,noise[i][j]));
      leds.drawPixel(i,j, CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]));
      // You can also explore other ways to constrain the hue used, like below
      // leds[XY(i,j)] = CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]);
    }
  }
  ihue+=1;
}

