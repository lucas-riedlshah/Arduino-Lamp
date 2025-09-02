#include "FastLED.h" // version 3.9.20
#include <EEPROM.h>
#include "shared_config.h"
#include "candle_mode.h"
#include "noise_mode.h"
#include "gradient_mode.h"
#define MODE_ADDR 0

CRGB leds[NUM_LEDS * NUM_STRIPS];

uint8_t mode = 0;

void setup() {
  Serial.begin(9600);

  FastLED.addLeds< WS2812B, DATA_START + 0, GRB >(leds, 0 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 11, GRB >(leds, 1 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 3, GRB >(leds, 2 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 5, GRB >(leds, 3 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 8, GRB >(leds, 4 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 4, GRB >(leds, 5 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 10, GRB >(leds, 6 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 1, GRB >(leds, 7 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 2, GRB >(leds, 8 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 6, GRB >(leds, 9 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 7, GRB >(leds, 10 * NUM_LEDS, NUM_LEDS);
  FastLED.addLeds< WS2812B, DATA_START + 9, GRB >(leds, 11 * NUM_LEDS, NUM_LEDS);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 19000);

  randomSeed(analogRead(8));
  initNoise();

  FastLED.setBrightness(0);
  fill_solid(leds, NUM_LEDS * NUM_STRIPS, CRGB::Black);
  FastLED.show();
  FastLED.setBrightness(100);

  mode = EEPROM.read(MODE_ADDR);
  mode = (mode + 1) % 4;
  EEPROM.write(MODE_ADDR, mode);
}

void loop() {
  switch (mode) {
    case 0:
      paintCandleFlicker(CHSV(40, 255, 190), CHSV(40, 150, 255));
      break;
    case 1:
      paintNoise();
      break;
    case 2:
      paintGradient(CHSV(43, 255, 255), CHSV(220, 255, 255));
      break;
    case 3:
    default:
      paintMovingGradient(CHSV(43, 255, 255), CHSV(220, 255, 255));
      break;
  }
}
