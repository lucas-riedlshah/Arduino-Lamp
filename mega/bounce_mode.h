#ifndef BOUNCE_MODE_H
#define BOUNCE_MODE_H


#include "ModeInterface.h"
#include <FastLED.h>

class BounceMode : public ModeInterface {
public:
    BounceMode(CRGB ringColor = CRGB(80, 0, 80), CRGB fadeColor = CRGB(40, 0, 60));
    void setup() override;
    void loop() override;
    void cleanup() override;
    void setColors(CRGB ring, CRGB fade) { ringColor = ring; fadeColor = fade; }
private:
    static const int NUM_RINGS = 6;
    float positions[NUM_RINGS];
    int directions[NUM_RINGS];
    float speeds[NUM_RINGS];
    int ringSize;
    CRGB ringColor;
    CRGB fadeColor;
};

#endif // BOUNCE_MODE_H
