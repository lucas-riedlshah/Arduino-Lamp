#include "gradient_candle_mode.h"
#include "shared_config.h"
#include <FastLED.h>

GradientCandleMode::GradientCandleMode() {
    gradientOffset = 0.0;
    gradientSpeed = 0.01;
    // Use CHSV for higher saturation
    colorStart = CHSV(43, 255, 255); // vivid yellow
    colorEnd = CHSV(220, 255, 255);  // vivid pink
}

void GradientCandleMode::setup() {
    gradientOffset = 0.0;
}

void GradientCandleMode::loop() {
    // Animate moving gradient
    gradientOffset += gradientSpeed;
    if (gradientOffset >= 2.0) gradientOffset -= 2.0;

    const float stretch = 0.5; // Lower value for wider gradient
    for (int strip = 0; strip < NUM_STRIPS; strip++) {
        for (int led = 0; led < NUM_LEDS; led++) {
            float ledPos = ((float)led / (NUM_LEDS - 1)) * stretch + gradientOffset;
            if (ledPos >= 2.0) ledPos -= 2.0;
            float blendPos = ledPos < 1.0 ? ledPos : 2.0 - ledPos;
            if (blendPos < 0.0) blendPos = 0.0;
            if (blendPos > 1.0) blendPos = 1.0;
            uint8_t blendAmount = (uint8_t)(blendPos * 255);
            CRGB gradColor = blend(colorStart, colorEnd, blendAmount);
            // Candle flicker: randomly modulate brightness
            uint8_t flicker = 180 + random8(75); // 180-255
            gradColor.nscale8_video(flicker);
            leds[strip * NUM_LEDS + led] = gradColor;
        }
    }
    FastLED.show();
    uint8_t d1 = random8(20, 40);
    uint8_t d2 = random8(20, 40);
    delay(d1 < d2 ? d1 : d2);
}

void GradientCandleMode::cleanup() {
    // Optionally clear LEDs
}
