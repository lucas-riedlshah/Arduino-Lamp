#ifndef GRADIENT_MODE_H
#define GRADIENT_MODE_H

#include "shared_config.h"
#include "ModeInterface.h"

class GradientMode : public ModeInterface {
    CHSV colorStart, colorEnd;
    float gradientOffset = 0.0;
    const float gradientSpeed = 0.02;
    bool moving;
public:
    GradientMode(CHSV start, CHSV end, bool movingGradient = false)
        : colorStart(start), colorEnd(end), moving(movingGradient) {}
    void setup() override;
    void loop() override;
    void cleanup() override {}
    ~GradientMode() {}
};

#endif
