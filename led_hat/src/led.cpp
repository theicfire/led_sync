
#include <Arduino.h>
#include <FastLED.h>

#include "time.h"
#include "friend.h"

#define LED_PIN     5
#define NUM_LEDS    300
//#define BRIGHTNESS  100
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

#define UPDATES_PER_SECOND 25

//extern const TProgmemPalette16 only_red PROGMEM;

// Gradients from http://fastled.io/tools/paletteknife/
DEFINE_GRADIENT_PALETTE( pink1 ) {
    0, 247,195,194,
  255, 255, 0, 255};

DEFINE_GRADIENT_PALETTE( pink2 ) {
    0, 255, 255, 255,
    128, 255, 0, 128,
  255, 255, 0, 255};

DEFINE_GRADIENT_PALETTE( pink3 ) {
    0, 255, 255, 255,
    128, 255, 0, 128,
  255, 128, 0, 128};

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


DEFINE_GRADIENT_PALETTE( es_seadreams_04_gp ) {
    0, 137,229,240,
   68,   2, 59,119,
  158,  16,133,205,
  252,  95,233,245,
  255,  95,233,245};

DEFINE_GRADIENT_PALETTE( alarm_p1_0_5_gp ) {
    0, 182,255,168,
   12, 206,255, 63,
   25, 255,231, 36,
   38, 255,168, 21,
   51, 255,114, 12,
   63, 255, 68,  6,
   76, 255, 35,  3,
   89, 255, 13,  1,
  102, 255,  2,  1,
  114, 255,  1,  6,
  127, 255,  1, 52,
  140, 255,  1,156,
  153, 190,  1,255,
  165,  75,  1,255,
  178,  19,  1,255,
  191,   1,  1,255,
  204,   1,  8,255,
  216,   1, 44,255,
  229,   1,108,255,
  242,   1,199,255,
  255,   1,255,186};


DEFINE_GRADIENT_PALETTE( BrBG_07_gp ) {
    0,  53, 20,  1,
   36,  53, 20,  1,
   36, 165,117, 25,
   72, 165,117, 25,
   72, 232,207,130,
  109, 232,207,130,
  109, 229,233,230,
  145, 229,233,230,
  145, 133,211,194,
  182, 133,211,194,
  182,  17,118, 95,
  218,  17,118, 95,
  218,   1, 33, 21,
  255,   1, 33, 21};


// Have green
DEFINE_GRADIENT_PALETTE( bhw1_05_gp ) {
    0,   1,221, 53,
  255,  73,  3,178};

DEFINE_GRADIENT_PALETTE( bhw1_26_gp ) {
    0, 107,  1,205,
   35, 255,255,255,
   73, 107,  1,205,
  107,  10,149,210,
  130, 255,255,255,
  153,  10,149,210,
  170,  27,175,119,
  198,  53,203, 56,
  207, 132,229,135,
  219, 255,255,255,
  231, 132,229,135,
  252,  53,203, 56,
  255,  53,203, 56};

DEFINE_GRADIENT_PALETTE( bhw1_sunconure_gp ) {
    0,  20,223, 13,
  160, 232, 65,  1,
  252, 232,  5,  1,
  255, 232,  5,  1};

DEFINE_GRADIENT_PALETTE( bhw2_turq_gp ) {
    0,   1, 33, 95,
   38,   1,107, 37,
   76,  42,255, 45,
  127, 255,255, 45,
  178,  42,255, 45,
  216,   1,107, 37,
  255,   1, 33, 95};

DEFINE_GRADIENT_PALETTE( bhw2_10_gp ) {
    0,   0, 12,  0,
   61, 153,239,112,
  127,   0, 12,  0,
  165, 106,239,  2,
  196, 167,229, 71,
  229, 106,239,  2,
  255,   0, 12,  0};

DEFINE_GRADIENT_PALETTE( bhw3_61_gp ) {
    0,  14,  1, 27,
   48,  17,  1, 88,
  104,   1, 88,156,
  160,   1, 54, 42,
  219,   9,235, 52,
  255, 139,235,233};

DEFINE_GRADIENT_PALETTE( bhw4_029_gp ) {
    0,  28,  7, 75,
   43,  73, 22, 74,
   79, 177,146,197,
  122,  21, 72,137,
  165,  15,184, 75,
  255, 224,205,  4};

DEFINE_GRADIENT_PALETTE( aspectcolr_gp ) {
    0, 255,255,255,
    0, 255,255,  0,
   63,   0,255,  0,
  127,   0,255,255,
  191, 255,  0,  0,
  255, 255,255,  0};

DEFINE_GRADIENT_PALETTE( only_red ) {
    0, 255,0,0,
    255, 255,0,0};

DEFINE_GRADIENT_PALETTE( only_green ) {
    0, 0,255,0,
    255, 0,255,0 };

DEFINE_GRADIENT_PALETTE( only_blue ) {
    0, 0,0,255,
    255, 0,0,255 };

DEFINE_GRADIENT_PALETTE( only_yellow ) {
    0, 255,240,0,
    255, 255,240,0 };

DEFINE_GRADIENT_PALETTE( only_purple ) {
    0, 255,0,255,
    255, 255,0,255 };

DEFINE_GRADIENT_PALETTE( only_teal ) {
    0, 0,240,255,
    255, 0,240,255 };

DEFINE_GRADIENT_PALETTE( only_orange ) {
    0, 255,128,0,
    255, 255,128,0 };

void runPaletteGradient(int index, uint8_t brightness)
{
    uint8_t colorIndex = index;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void runPaletteFullFade(int index) {
    uint8_t brightness = 255;
    uint8_t colorIndex = index / 4; // slow down

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


void ChooseLonerGradient(unsigned long time, int speed)
{
    uint8_t secondHand = (time / 1000) % (13 * speed);
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        Serial.print("Loner secondHand: "); Serial.println(secondHand);
    }


    if( secondHand <= 1 * speed)  { targetPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 1 * speed + 3)  { targetPalette = only_red;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 3 * speed)  { targetPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 3 * speed + 3)  { targetPalette = only_blue;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 5 * speed)  { targetPalette = fireandice_gp;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 5 * speed + 3)  { targetPalette = only_yellow;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 7 * speed)  { targetPalette = sky_45_gp;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 7 * speed + 3)  { targetPalette = only_purple;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 9 * speed)  { targetPalette = es_seadreams_04_gp;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 9 * speed + 3)  { targetPalette = only_teal;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 11 * speed)  { targetPalette = alarm_p1_0_5_gp;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 11 * speed + 3)  { targetPalette = only_orange;           currentBlending = LINEARBLEND; }
    else if( secondHand <= 13 * speed)  { targetPalette = BrBG_07_gp;           currentBlending = LINEARBLEND; }
}

void ChooseFriendGradient(unsigned long time, int speed)
{
    uint8_t secondHand = (time / 1000) % (5 * speed + 3);
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        Serial.print("Friend secondHand: "); Serial.println(secondHand);
    }
    if( secondHand <= 1 * speed)  { targetPalette = pink1;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 1 * speed + 3)  { targetPalette = only_green;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 3 * speed)  { targetPalette = pink2;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 3 * speed + 3)  { targetPalette = only_red;         currentBlending = LINEARBLEND; }
    else if( secondHand <= 5 * speed)  { targetPalette = pink3;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 5 * speed + 3)  { targetPalette = only_blue;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 7 * speed)  { targetPalette = bhw1_sunconure_gp;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 7 * speed + 3)  { targetPalette = only_yellow;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 9 * speed)  { targetPalette = bhw2_turq_gp;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 9 * speed + 3)  { targetPalette = only_purple;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 11 * speed)  { targetPalette = bhw2_10_gp;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 11 * speed + 3)  { targetPalette = only_teal;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 13 * speed)  { targetPalette = bhw3_61_gp;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 13 * speed + 3)  { targetPalette = only_orange;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 15 * speed)  { targetPalette = bhw4_029_gp;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 15 * speed + 3)  { targetPalette = only_green;         currentBlending = LINEARBLEND; }
    //else if( secondHand <= 17 * speed)  { targetPalette = aspectcolr_gp;         currentBlending = LINEARBLEND; }
}

void LED_Init() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    //FastLED.setBrightness(BRIGHTNESS);

    targetPalette = pink1;
    currentPalette = pink1;
    currentBlending = LINEARBLEND;
}

uint8_t get_brightness(bool friendExists) {
  return 10;
    uint8_t friend_brightness = 30;
    uint8_t loner_brightness = 10;
    static unsigned long start_brightness_move = 0;
    static bool didFriendExist = false;
    if (friendExists != didFriendExist) {
      start_brightness_move = millis();
      didFriendExist = friendExists;
    }
    unsigned long brightness_move_fraction_long = (millis() - start_brightness_move) * 255 / 1000;
    uint8_t brightness_move_fraction = 255;
    if (brightness_move_fraction_long < 250) {
      brightness_move_fraction = brightness_move_fraction_long;
    }
    if (friendExists) {
      return ((unsigned long) (friend_brightness - loner_brightness)) * brightness_move_fraction / 255 + loner_brightness;
    } else {
      return friend_brightness - ((unsigned long) (friend_brightness - loner_brightness)) * brightness_move_fraction / 255;
    }

}

void LED_Update()
{
    static unsigned long last_brightness_update_time = 0;

    // Incrementing index gives a shifting effect on the LEDs. Overflow uint8_t on purpose.
    uint8_t index = Time_GetTime() / (1000 / UPDATES_PER_SECOND);
    printf("Index %d\n", index);

    int brightness_change = (Time_GetTime() - last_brightness_update_time) / (1000 / 25);
    // if (friendExists) {
    if (true) {
      ChooseFriendGradient(Time_GetTime(), 5);
    } else {
      ChooseLonerGradient(Time_GetTime(), 5);
    }
    nblendPaletteTowardPalette( currentPalette, targetPalette, 48);


    uint8_t brightness = get_brightness(true);
    //Serial.println(brightness);
    runPaletteGradient(index, brightness);

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



//const TProgmemPalette16 only_red PROGMEM =
//{
    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,

    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,

    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,

    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,
    //CRGB::Red,
//};


