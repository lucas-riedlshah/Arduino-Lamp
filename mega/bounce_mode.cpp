

#include "bounce_mode.h"
#include "shared_config.h"
#include <FastLED.h>

// Helper to calculate brightness for ring and fade
static uint8_t getBounceBrightness(int idx, int center, int ringSize, int fadeLen) {
    int dist = abs(idx - center);
    if (dist <= ringSize / 2) {
        // Inside ring: full blend, fade toward edge
        return 255 - dist * (180 / (ringSize / 2 + 1));
    } else {
        // Outside ring: fade out
        int fadeDist = dist - ringSize / 2;
        int b = 180 - fadeDist * (180 / (fadeLen + 1));
        return b < 10 ? 0 : b;
    }
}

BounceMode::BounceMode(CRGB ring, CRGB fade) : ringColor(ring), fadeColor(fade) {
    ringSize = 3; // Number of LEDs in the ring
}

void BounceMode::setup() {
    for (int i = 0; i < NUM_RINGS; i++) {
        positions[i] = random(NUM_LEDS - ringSize);
        directions[i] = random(2) == 0 ? 1 : -1;
        speeds[i] = random(1, 3); // speed between 0.5 and 5
    }
}

void BounceMode::loop() {
    // 1. Set background to fadeColor
    for (int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
        leds[i] = fadeColor;
    }

    // 2. Fade all LEDs slightly for trailing effect
    for (int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
        leds[i].fadeToBlackBy(40); // Lower value = longer trail
    }

    // 3. Draw 5 bouncing rings at random positions
    for (int strip = 0; strip < NUM_STRIPS; strip++) {
        for (int r = 0; r < NUM_RINGS; r++) {
            int pos = (int)positions[r];
            // Draw main ring
            for (int i = 0; i < ringSize; i++) {
                int idx = pos + i;
                if (idx >= 0 && idx < NUM_LEDS) {
                    leds[strip * NUM_LEDS + idx] = ringColor;
                }
            }
            // Draw faded before
            int beforeIdx = pos - 1;
            if (beforeIdx >= 0 && beforeIdx < NUM_LEDS) {
                CRGB faded = ringColor; faded.nscale8_video(128);
                leds[strip * NUM_LEDS + beforeIdx] = faded;
            }
            // Draw faded after
            int afterIdx = pos + ringSize;
            if (afterIdx >= 0 && afterIdx < NUM_LEDS) {
                CRGB faded = ringColor; faded.nscale8_video(128);
                leds[strip * NUM_LEDS + afterIdx] = faded;
            }
        }
    }

    // 4. Move each ring at its own speed
    for (int r = 0; r < NUM_RINGS; r++) {
        positions[r] += directions[r] * speeds[r];
        if (positions[r] <= 0) {
            positions[r] = 0;
            directions[r] *= -1;
        }
        if (positions[r] + ringSize >= NUM_LEDS) {
            positions[r] = NUM_LEDS - ringSize - 1;
            directions[r] *= -1;
        }
    }

    FastLED.show();
    delay(10);
}

void BounceMode::cleanup() {
    // Optionally clear LEDs
}
