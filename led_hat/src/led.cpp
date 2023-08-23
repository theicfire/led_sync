
#include <Arduino.h>
#include <FastLED.h>
#include <stdlib.h>
#include <string>

#include <map>
#include "FastLED_RGBW.h"
#include "time.h"
#include "friend.h"
using namespace std;

#define LED_PIN1     5
#define LED_PIN2     6
#define NUM_LEDS    189


// SK6812 timing more closely matches WS2811
#define LED_TYPE    WS2811
// SK2812 expects RGBW, WS2812B expects GRB
#define COLOR_ORDER RGB

// FastLED with RGBW
CRGBW leds1[NUM_LEDS];
CRGB *ledsRGB1 = (CRGB *) &leds1[0];
CRGBW leds2[NUM_LEDS];
CRGB *ledsRGB2 = (CRGB *) &leds2[0];


CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

#define UPDATES_PER_SECOND 50

struct RGBW {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;
};

void LED_Init() {
    FastLED.addLeds<LED_TYPE, LED_PIN1, COLOR_ORDER>(ledsRGB1, getRGBWsize(NUM_LEDS));
    FastLED.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(ledsRGB2, getRGBWsize(NUM_LEDS));

    targetPalette = RainbowColors_p;
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

const int DEFAULT_HUE        = 013;
const int DEFAULT_SATURATION = 255;
const int DEFAULT_BRIGHTNESS = 128;

/// speed is currently inversely proportional to the desired speed
/// 0 is the fastest, 6 is nice and slow, too high gets choppy
void updateHuesForWaves(int originalHue, int targetHue, int width, int speed) {
  int hues[NUM_LEDS];
  std::fill_n(hues, NUM_LEDS, DEFAULT_HUE);

  unsigned long currentFrame = millis() / (1000 / UPDATES_PER_SECOND);
  if (currentFrame % speed != 0) {
    return;
  }

  int currentCenter = currentFrame % NUM_LEDS;

  float hueSlope = float(targetHue - originalHue) / (width / 2.0);
  for (int i = 0; i <= width / 2; i++) {
    int leftIndex  = ((currentCenter - i) % NUM_LEDS + NUM_LEDS) % NUM_LEDS;
    int rightIndex = ((currentCenter + i) % NUM_LEDS + NUM_LEDS) % NUM_LEDS;

    hues[leftIndex]  += hueSlope * abs((width / 2) - i);
    if (leftIndex != rightIndex) {
      hues[rightIndex]  += hueSlope * abs((width / 2) - i);
    }
  }

  for (int i = 0; i < NUM_LEDS; i++) {
      leds1[i] = CHSV(hues[i], DEFAULT_SATURATION, DEFAULT_BRIGHTNESS);
      leds2[i] = CHSV(hues[i], DEFAULT_SATURATION, DEFAULT_BRIGHTNESS);
  }
  FastLED.show();
}

void updateHuesForRandom() {
  for (int i = 0; i < NUM_LEDS; i++) {
      leds1[i] = CHSV(random(255), DEFAULT_SATURATION, DEFAULT_BRIGHTNESS);
      leds2[i] = CHSV(random(255), DEFAULT_SATURATION, DEFAULT_BRIGHTNESS);
  }
  FastLED.show();
}

int morseCurCharInPhraseIndex = 0;
int morseCurCharInMorseLetterIndex = 0;
unsigned long startTimeOfMorseCurChar = 0;

std::map<char, std::string> morseMap = {{'0', "-----"},
{'1', ".----"},
{'2', "..---"},
{'3', "...--"},
{'4', "....-"},
{'5', "....."},
{'6', "-...."},
{'7', "--..."},
{'8', "---.."},
{'9', "----."},
{'a', ".-"},
{'b', "-..."},
{'c', "-.-."},
{'d', "-.."},
{'e', "."},
{'f', "..-."},
{'g', "--."},
{'h', "...."},
{'i', ".."},
{'j', ".---"},
{'k', "-.-"},
{'l', ".-.."},
{'m', "--"},
{'n', "-."},
{'o', "---"},
{'p', ".--."},
{'q', "--.-"},
{'r', ".-."},
{'s', "..."},
{'t', "-"},
{'u', "..-"},
{'v', "...-"},
{'w', ".--"},
{'x', "-..-"},
{'y', "-.--"},
{'z', "--.."},
{'.', ".-.-.-"},
{',', "--..--"},
{'?', "..--.."},
{'!', "-.-.--"},
{'-', "-....-"},
{'/', "-..-."},
{'@', ".--.-."},
{'(', "-.--."},
{')', "-.--.-"}};

enum SpaceMode {
  END_OF_MORSE_LETTER,
  END_OF_LETTER,
  END_OF_WORD,
  NOT_END
};

SpaceMode mode = NOT_END;

void updateForMorseCode(string phrase, int timeUnit) {
  if (startTimeOfMorseCurChar == 0) {
    startTimeOfMorseCurChar = millis();
  }

  char curChar = phrase[morseCurCharInPhraseIndex];

  int morseCharTimeUnits;
  bool isOn = false;
  if (mode == NOT_END) {
      if (curChar == ' ') {
        // SPACES ARE 7, but lead into an END_OF_LETTER WHICH IS 3 seconds, GET FUCKED
        morseCharTimeUnits = 3;
      } else {
        string morseCode = morseMap[curChar];
        char morseChar = morseCode[morseCurCharInMorseLetterIndex];
        if (morseChar == '-') {
          morseCharTimeUnits = 3;
        } else if (morseChar == '.') {
          morseCharTimeUnits = 1;
        }
        isOn = true; 
      }
  } else if (mode == END_OF_MORSE_LETTER) {
      morseCharTimeUnits = 1;
  } else if (mode == END_OF_LETTER) {
      morseCharTimeUnits = 3;
  } else if (mode == END_OF_WORD) {
      morseCharTimeUnits = 7;
  }

  if (millis() - startTimeOfMorseCurChar > morseCharTimeUnits*timeUnit) {
      startTimeOfMorseCurChar = millis();
    if (mode != NOT_END) {
      mode = NOT_END;
      return;
    }

    mode = END_OF_MORSE_LETTER;
    ++morseCurCharInMorseLetterIndex;

    // End of letter
    if ((curChar == ' ') || (morseCurCharInMorseLetterIndex == morseMap[curChar].size())) {
      morseCurCharInMorseLetterIndex = 0;
      mode = END_OF_LETTER;

      ++morseCurCharInPhraseIndex;

      // End of phrase
      if (morseCurCharInPhraseIndex == phrase.size()) {
        morseCurCharInPhraseIndex = 0;
        mode = END_OF_WORD;
      }
      return;
    }
  }

  for (int i; i < NUM_LEDS; i++) {
    if (isOn) {
        leds1[i] = CHSV(DEFAULT_HUE, DEFAULT_SATURATION, DEFAULT_BRIGHTNESS);
        leds2[i] = CHSV(DEFAULT_HUE, DEFAULT_SATURATION, DEFAULT_BRIGHTNESS);
    } else {
        leds1[i] = CHSV(DEFAULT_HUE, DEFAULT_SATURATION, 0);
        leds2[i] = CHSV(DEFAULT_HUE, DEFAULT_SATURATION, 0);
    }
  }
  FastLED.show();
}

void LED_Update()
{
  // Currently have to choose which function to call to define a mode
  updateHuesForWaves(DEFAULT_HUE, 0, 80, 10);
  // updateForMorseCode("it was better next year", 250);
  // updateHuesForRandom();
}