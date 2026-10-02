#include "strip_test_mode.h"
#include "shared_config.h"

void StripTestMode::setup() {
    fill_solid(leds, NUM_STRIPS * NUM_LEDS, CRGB::Black);

    // A clockwise staircase: 4 LEDs on the first strip, then 8, ... 48.
    for (uint8_t position = 0; position < NUM_STRIPS; ++position) {
        const uint16_t barHeight = (uint16_t)(position + 1) * NUM_LEDS / NUM_STRIPS;
        fill_solid(leds + (uint16_t)position * NUM_LEDS, barHeight, CRGB::White);
    }
    FastLED.show();
}
