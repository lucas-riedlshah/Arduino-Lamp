#ifndef CANDLE_MODE_H
#define CANDLE_MODE_H

#include "shared_config.h"

extern uint8_t steady, flicker, turb;
extern uint8_t v;
extern CHSV color;
extern int f;
extern int g;

#define CANDLE_STEADY_FREQ 0.1
#define CANDLE_FLICKER_FREQ 2
#define CANDLE_TURB_FREQ 0.1
#define CANDLE_FLICKER_THRESHOLD 80

void paintCandleFlicker(CHSV color1, CHSV color2);

#endif
