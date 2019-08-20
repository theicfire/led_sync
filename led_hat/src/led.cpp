
#include <Arduino.h>
#include <FastLED.h>

#include "time.h"
#include "friend.h"

#define LED_PIN     5
#define NUM_LEDS    300
#define BRIGHTNESS  20
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

#define UPDATES_PER_SECOND 50

extern const TProgmemPalette16 only_red PROGMEM;

DEFINE_GRADIENT_PALETTE( fireandice_gp ) {
    0,  80,  2,  1,
   51, 206, 15,  1,
  101, 242, 34,  1,
  153,  16, 67,128,
  204,   2, 21, 69,
  255,   1,  2,  4};

DEFINE_GRADIENT_PALETTE( sky_45_gp ) {
    0, 249,205,  4,
   51, 255,239,123,
   87,   5,141, 85,
  178,   1, 26, 43,
  255,   0,  2, 23};


void runPaletteGradient(int index)
{
    uint8_t brightness = 255;
    uint8_t colorIndex = index;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void runPaletteFullFade(int index) {
    uint8_t brightness = 255;
    uint8_t colorIndex = index;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

void runMeteorRain(int index, byte red, byte green, byte blue) {
    uint8_t meteorSize = 10;
    uint8_t meteorTrailDecay = 64;
    // 2x leds to have the tail of the meteor work correctly
    int i = index % (NUM_LEDS * 2);
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
        if(random(10)>5) {
            leds[j].fadeToBlackBy( meteorTrailDecay );
        }
    }

    // draw meteor
    for (int i = index % 100; i < NUM_LEDS; i+= 100) {
      for(int j = 0; j < meteorSize; j++) {
          if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
            leds[i - j] = ColorFromPalette(currentPalette, index, 0xFF, currentBlending);
          }
      }
    }
}


void ChooseLonerGradient(unsigned long time)
{
    uint8_t secondHand = (time / 1000) % 30;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        Serial.print("secondHand: "); Serial.println(secondHand);
    }

    if( secondHand <= 5)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 10)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 15)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 20)  { currentPalette = fireandice_gp;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 25)  { currentPalette = sky_45_gp;           currentBlending = LINEARBLEND; }
}

void ChooseFriendGradient(unsigned long time)
{
    uint8_t secondHand = (time / 1000) % 30;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        Serial.print("secondHand: "); Serial.println(secondHand);
    }
    currentPalette = only_red;
}

void LED_Init() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void LED_Update()
{
    uint8_t secondHand = (Time_GetTime() / 1000) % 60;
    if (friendExists) {
      ChooseFriendGradient(Time_GetTime());
    } else {
      ChooseLonerGradient(Time_GetTime());
    }

    uint8_t index = Time_GetTime() / (1000 / UPDATES_PER_SECOND);

    if (secondHand <= 30) {
      runPaletteGradient(index);
    } else if (secondHand <= 40) {
      runPaletteFullFade(index);
    } else {
      runMeteorRain(index, 0xFF, 0, 0);
    }

    FastLED.show();
}

//// This function fills the palette with totally random colors.
//void SetupTotallyRandomPalette()
//{
    //for( int i = 0; i < 16; i++) {
        //currentPalette[i] = CHSV( random8(), 255, random8());
    //}
//}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
//void SetupBlackAndWhiteStripedPalette()
//{
    //// 'black out' all 16 palette entries...
    //fill_solid( currentPalette, 16, CRGB::Black);
    //// and set every fourth one to white.
    //currentPalette[0] = CRGB::White;
    //currentPalette[4] = CRGB::White;
    //currentPalette[8] = CRGB::White;
    //currentPalette[12] = CRGB::White;
    
//}

//// This function sets up a palette of purple and green stripes.
//void SetupPurpleAndGreenPalette()
//{
    //CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    //CRGB green  = CHSV( HUE_GREEN, 255, 255);
    //CRGB black  = CRGB::Black;
    
    //currentPalette = CRGBPalette16(
                                   //green,  green,  black,  black,
                                   //purple, purple, black,  black,
                                   //green,  green,  black,  black,
                                   //purple, purple, black,  black );
//}



const TProgmemPalette16 only_red PROGMEM =
{
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,

    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,

    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,

    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
};


