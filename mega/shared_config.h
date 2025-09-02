#ifndef SHARED_CONFIG_H
#define SHARED_CONFIG_H

#include "FastLED.h"

// Common constants
#define NUM_LEDS 48
#define NUM_STRIPS 12
#define DATA_START 2
#define SPEED 20
#define SCALE 5
#define STEPS 5

// Shared variables
extern CRGB leds[NUM_LEDS * NUM_STRIPS];

// Shared functions
uint8_t mynoise(uint16_t x, uint16_t y);

#endif
