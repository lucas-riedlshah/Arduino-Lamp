#include "life_grid.h"

bool LifeGrid::bitIsAlive(const uint8_t* state, uint8_t strip, uint8_t row) {
    const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
    return (state[index >> 3] & (1 << (index & 7))) != 0;
}

void LifeGrid::setBitAlive(uint8_t* state, uint8_t strip, uint8_t row) {
    const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
    state[index >> 3] |= 1 << (index & 7);
}

bool LifeGrid::isAlive(uint8_t strip, uint8_t row) const {
    return bitIsAlive(cells, strip, row);
}

void LifeGrid::setAlive(uint8_t strip, uint8_t row) {
    setBitAlive(cells, strip, row);
}

uint8_t LifeGrid::countNeighbors(uint8_t strip, uint8_t row) const {
    uint8_t neighbors = 0;
    for (int8_t dx = -1; dx <= 1; ++dx) {
        int8_t neighborStrip = (int8_t)strip + dx;
        if (neighborStrip < 0) neighborStrip = NUM_STRIPS - 1;
        if (neighborStrip >= NUM_STRIPS) neighborStrip = 0;
        for (int8_t dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int8_t neighborRow = (int8_t)row + dy;
            if (neighborRow < 0) neighborRow = NUM_LEDS - 1;
            if (neighborRow >= NUM_LEDS) neighborRow = 0;
            if (bitIsAlive(cells, neighborStrip, neighborRow)) ++neighbors;
        }
    }
    return neighbors;
}

void LifeGrid::clear() {
    memset(cells, 0, sizeof(cells));
    memset(nextCells, 0, sizeof(nextCells));
}

uint16_t LifeGrid::liveCount() const {
    uint16_t count = 0;
    for (uint16_t i = 0; i < STATE_BYTES; ++i) {
        uint8_t bits = cells[i];
        while (bits != 0) {
            count += bits & 1;
            bits >>= 1;
        }
    }
    return count;
}

void LifeGrid::calculateNext() {
    memset(nextCells, 0, sizeof(nextCells));
    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            const uint8_t neighbors = countNeighbors(strip, row);
            const bool alive = isAlive(strip, row);
            const bool survives = neighbors == 3 || (alive && neighbors == 2);
            if (!survives) continue;
            setBitAlive(nextCells, strip, row);
        }
    }
}

bool LifeGrid::hasChanged() const {
    return memcmp(cells, nextCells, sizeof(cells)) != 0;
}

void LifeGrid::commit() {
    memcpy(cells, nextCells, sizeof(cells));
}
