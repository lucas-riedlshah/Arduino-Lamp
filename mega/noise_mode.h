#ifndef NOISE_MODE_H
#define NOISE_MODE_H

#include "shared_config.h"
#include "ModeInterface.h"

class NoiseMode : public ModeInterface {
    CRGB*** noise = nullptr;
    uint16_t roff, goff, boff, z;
public:
    void setup() override;
    void loop() override;
    void cleanup() override;
    ~NoiseMode() { cleanup(); }
};

#endif
