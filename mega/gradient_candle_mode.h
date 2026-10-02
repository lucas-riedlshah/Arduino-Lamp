#ifndef GRADIENT_CANDLE_MODE_H
#define GRADIENT_CANDLE_MODE_H

#include "ModeInterface.h"
#include <FastLED.h>

class GradientCandleMode : public ModeInterface {
public:
    GradientCandleMode();
    void setup() override;
    void loop() override;
    void cleanup() override;
private:
    float gradientOffset;
    float gradientSpeed;
    CRGB colorStart;
    CRGB colorEnd;
};

#endif // GRADIENT_CANDLE_MODE_H
