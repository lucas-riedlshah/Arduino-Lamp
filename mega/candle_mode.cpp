#include "candle_mode.h"
#include "shared_config.h"

// Candle flicker variables
uint8_t steady, flicker, turb;
uint8_t v;
CHSV color;
int f = 0;
int g;

void paintCandleFlicker(CHSV color1, CHSV color2) {
  steady = mynoise(10000, millis() * CANDLE_STEADY_FREQ);
  flicker = mynoise(20000, millis() * CANDLE_FLICKER_FREQ);
  turb = mynoise(30000, millis() * CANDLE_TURB_FREQ);

  v = (turb < CANDLE_FLICKER_THRESHOLD ? 0.6 : 1) * steady +
      (turb < CANDLE_FLICKER_THRESHOLD ? 0.4 : 0) * flicker;

  color = blend(color1, color2, v);

  f += 1;
  g = f & 1;
  for (int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
    if ((i & 1) == g) {
      leds[i] = color;
    }
  }
  FastLED.show();
}
