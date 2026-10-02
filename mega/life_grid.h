#ifndef LIFE_GRID_H
#define LIFE_GRID_H

#include "shared_config.h"
#include <stdint.h>
#include <string.h>

// Packed Conway state, kept separate from LifeMode's timing, color, and output.
class LifeGrid {
public:
    static const uint16_t CELL_COUNT = NUM_STRIPS * NUM_LEDS;
    static const uint16_t STATE_BYTES = (CELL_COUNT + 7) / 8;

    bool isAlive(uint8_t strip, uint8_t row) const;
    void setAlive(uint8_t strip, uint8_t row);
    void clear();
    uint16_t liveCount() const;
    void calculateNext();
    bool hasChanged() const;
    void commit();

private:
    uint8_t cells[STATE_BYTES];
    uint8_t nextCells[STATE_BYTES];

    static bool bitIsAlive(const uint8_t* state, uint8_t strip, uint8_t row);
    static void setBitAlive(uint8_t* state, uint8_t strip, uint8_t row);
    uint8_t countNeighbors(uint8_t strip, uint8_t row) const;
};

#endif
