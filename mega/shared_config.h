#ifndef SHARED_CONFIG_H
#define SHARED_CONFIG_H

#include "FastLED.h"

// Common constants
#define NUM_LEDS 48
#define NUM_STRIPS 12
#define SPEED 20
#define SCALE 5
#define STEPS 5

// Data pins in physical clockwise order, starting at the former red test strip.
constexpr uint8_t stripPinsByPosition[NUM_STRIPS] = {
    2, 6, 13, 9, 5, 12, 7, 3, 4, 11, 8, 10
};

// Shared variables
extern CRGB leds[NUM_LEDS * NUM_STRIPS];

// Shared functions
uint8_t mynoise(uint16_t x, uint16_t y);

#endif
