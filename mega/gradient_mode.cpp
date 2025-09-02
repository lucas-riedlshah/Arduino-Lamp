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
                float fadePos;
                if (ledPos <= 1.0) {
                    fadePos = ledPos;
                } else {
                    float compressedPos = (ledPos - 1.0) * 4.0;
                    fadePos = 1.0 - compressedPos;
                    if (fadePos < 0.0) fadePos = 0.0;
                }
                fadePos = fadePos * fadePos * (3.0 - 2.0 * fadePos);
                uint8_t blendAmount = (uint8_t)(fadePos * 255);
                leds[strip * NUM_LEDS + led] = blend(colorStart, colorEnd, blendAmount);
            }
        }
        FastLED.show();
        delay(min(random(0, 500), random(0, 500)));
    } else {
        // Static gradient
        for (int strip = 0; strip < NUM_STRIPS; strip++) {
            for (int led = 0; led < NUM_LEDS; led++) {
                uint8_t blendAmount = map(led, 0, NUM_LEDS - 1, 0, 255);
                leds[strip * NUM_LEDS + led] = blend(colorStart, colorEnd, blendAmount);
            }
        }
        FastLED.show();
        delay(30);
    }
}
