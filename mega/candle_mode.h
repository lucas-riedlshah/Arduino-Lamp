#ifndef CANDLE_MODE_H
#define CANDLE_MODE_H

#include "shared_config.h"
#include "ModeInterface.h"

#define CANDLE_STEADY_FREQ 0.1
#define CANDLE_FLICKER_FREQ 2
#define CANDLE_TURB_FREQ 0.1
#define CANDLE_FLICKER_THRESHOLD 80

class CandleMode : public ModeInterface {
    uint8_t steady, flicker, turb;
    uint8_t v;
    CHSV color;
    int f = 0;
    int g;
public:
    void setup() override;
    void loop() override;
    void cleanup() override {}
    ~CandleMode() {}
};

#endif
