#include "gradient_mode.h"
#include "shared_config.h"

void GradientMode::setup() {
    gradientOffset = 0.0;
}

void GradientMode::loop() {
    if (moving) {
        // Moving gradient
        gradientOffset += gradientSpeed;
        if (gradientOffset >= 2.0) {
            gradientOffset -= 2.0; // Loop back to 0 for full cycle
        }
        for (int strip = 0; strip < NUM_STRIPS; strip++) {
            for (int led = 0; led < NUM_LEDS; led++) {
                float ledPos = (float)led / (NUM_LEDS - 1) + gradientOffset;
                if (ledPos >= 2.0) {
                    ledPos -= 2.0;
                }
                float blendPos;
                if (ledPos < 1.0) {
                    blendPos = ledPos;
                } else {
                    blendPos = 2.0 - ledPos;
                }
                // Clamp blendPos between 0.0 and 1.0
                if (blendPos < 0.0) blendPos = 0.0;
                if (blendPos > 1.0) blendPos = 1.0;
                uint8_t blendAmount = (uint8_t)(blendPos * 255);
                leds[strip * NUM_LEDS + led] = blend(colorStart, colorEnd, blendAmount);
            }
        }
        FastLED.show();
        delay(min(random(0, 500), random(0, 500)));
    } else {
        // Static gradient
        for (int strip = 0; strip < NUM_STRIPS; strip++) {
            for (int led = 0; led < NUM_LEDS; led++) {
                float blendPos = (float)led / (NUM_LEDS - 1);
                uint8_t blendAmount = (uint8_t)(blendPos * 255);
                leds[strip * NUM_LEDS + led] = blend(colorStart, colorEnd, blendAmount);
            }
        }
        FastLED.show();
        delay(30);
    }
}
