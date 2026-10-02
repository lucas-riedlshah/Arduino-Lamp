#include "life_mode.h"
#include <math.h>
#include <string.h>

namespace {
struct LifePattern {
    uint8_t width;
    uint8_t height;
    const uint16_t* rows; // Bit 0 is the leftmost cell; each pattern fits in 16 columns.
    uint8_t weight;
};

// Add another pattern by defining its row masks and adding one entry here.
const uint16_t gliderRows[] = {0x2, 0x4, 0x7};             // .O. / ..O / OOO
const uint16_t spaceshipRows[] = {0x12, 0x1, 0x11, 0xF}; // .O..O / O.... / O...O / OOOO.
const uint16_t rPentominoRows[] = {0x6, 0x3, 0x2};        // .OO / OO. / .O.
const LifePattern patterns[] = {
    {3, 3, gliderRows, 6},
    {5, 4, spaceshipRows, 3},
    {3, 3, rPentominoRows, 1}
};
const uint8_t patternCount = sizeof(patterns) / sizeof(patterns[0]);

void patternOffset(const LifePattern& pattern, uint8_t x, uint8_t y,
                   uint8_t rotation, bool flipped, uint8_t& dx, uint8_t& dy) {
    if (flipped) x = pattern.width - 1 - x;
    switch (rotation) {
        case 0: dx = x; dy = y; break;
        case 1: dx = pattern.height - 1 - y; dy = x; break;
        case 2: dx = pattern.width - 1 - x; dy = pattern.height - 1 - y; break;
        default: dx = y; dy = pattern.width - 1 - x; break;
    }
}
}

bool LifeMode::isAlive(const uint8_t* state, uint8_t strip, uint8_t row) {
    const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
    return (state[index >> 3] & (1 << (index & 7))) != 0;
}

void LifeMode::setAlive(uint8_t* state, uint8_t strip, uint8_t row) {
    const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
    state[index >> 3] |= 1 << (index & 7);
}

uint8_t LifeMode::countNeighbors(uint8_t strip, uint8_t row) const {
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
            if (isAlive(cells, neighborStrip, neighborRow)) {
                ++neighbors;
            }
        }
    }
    return neighbors;
}

void LifeMode::clearGrid() {
    memset(cells, 0, sizeof(cells));
    memset(previousCells, 0, sizeof(previousCells));
    memset(hues, 0, sizeof(hues));
    memset(previousHues, 0, sizeof(previousHues));
    paused = true;
    fadeActive = false;
}

void LifeMode::render() {
    const unsigned long now = millis();
    const uint8_t offHue = newCellHue(now);
    lastOffHue = offHue;
    CRGB offColor(0, 0, 0);
    if (SHOW_DIM_OFF_CELLS) {
        offColor = CHSV(offHue, 240, OFF_CELL_BRIGHTNESS);
    }
    const unsigned long fadeElapsed = now - lastStep;
    const uint8_t fade = !fadeActive || fadeElapsed >= FADE_DURATION_MS
        ? 255 : (uint32_t)fadeElapsed * 255 / FADE_DURATION_MS;
    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            const bool alive = isAlive(cells, strip, row);
            const bool wasAlive = fadeActive && isAlive(previousCells, strip, row);
            if (alive) {
                CRGB litColor(0, 0, 0);
                litColor = CHSV(hues[index], 240, LIT_CELL_BRIGHTNESS);
                leds[index] = wasAlive || fade == 255
                    ? litColor : blend(offColor, litColor, fade);
            } else if (wasAlive && fade < 255) {
                CRGB previousColor(0, 0, 0);
                previousColor = CHSV(previousHues[index], 240, LIT_CELL_BRIGHTNESS);
                leds[index] = blend(previousColor, offColor, fade);
            } else {
                leds[index] = offColor;
            }
        }
    }
    FastLED.show();
}

uint16_t LifeMode::liveCellCount() const {
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

void LifeMode::schedulePatternInjection() {
    lastPatternInjection = millis();
    // The wait doubles smoothly for every PATTERN_RATE_HALF_LIFE_DOTS lit cells.
    const float waitMs = PATTERN_EMPTY_GRID_INTERVAL_MS *
        expf(0.69314718f * liveCellCount() / PATTERN_RATE_HALF_LIFE_DOTS);
    // Keep the random() bounds within signed 32-bit milliseconds.
    const float maxMeanMs = 2000000000.0f / (1.0f + PATTERN_INTERVAL_VARIATION);
    const float boundedWaitMs = waitMs < maxMeanMs ? waitMs : maxMeanMs;
    const long earliest = (long)(boundedWaitMs * (1.0f - PATTERN_INTERVAL_VARIATION) + 0.5f);
    const long latest = (long)(boundedWaitMs * (1.0f + PATTERN_INTERVAL_VARIATION) + 0.5f);
    patternInjectionDelay = random(earliest, latest + 1);
}

uint8_t LifeMode::newCellHue(unsigned long now) const {
    const unsigned long elapsed = (now - modeStartedAt) % NEW_CELL_HUE_CYCLE_MS;
    return (uint8_t)(PURPLE_HUE + elapsed * 256UL / NEW_CELL_HUE_CYCLE_MS);
}

void LifeMode::advanceHues(unsigned long now) {
    if (SHOW_DIM_OFF_CELLS && newCellHue(now) != lastOffHue) {
        renderPending = true;
    }
    const unsigned long elapsed = now - lastHueUpdate;
    lastHueUpdate = now;
    // One complete 8-bit hue rotation takes HUE_CYCLE_MS.
    const unsigned long phase = (elapsed % HUE_CYCLE_MS) * 256UL + hueRemainder;
    const uint8_t steps = phase / HUE_CYCLE_MS;
    hueRemainder = phase % HUE_CYCLE_MS;
    if (steps == 0) return;

    bool changed = false;
    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            if (isAlive(cells, strip, row)) {
                hues[(uint16_t)strip * NUM_LEDS + row] += steps;
                changed = true;
            }
        }
    }
    if (changed) renderPending = true;
}

void LifeMode::setup() {
    modeStartedAt = millis();
    clearGrid();
    render();
    lastRender = millis();
    renderPending = false;
    lastStep = lastRender;
    lastHueUpdate = lastStep;
    hueRemainder = 0;
    schedulePatternInjection();
}

void LifeMode::calculateNext() {
    memset(nextCells, 0, sizeof(nextCells));
    const uint8_t birthHue = newCellHue(millis());

    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            const uint8_t neighbors = countNeighbors(strip, row);
            const bool alive = isAlive(cells, strip, row);
            const bool survives = neighbors == 3 || (alive && neighbors == 2);
            if (!survives) continue;

            setAlive(nextCells, strip, row);
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            nextHues[index] = alive ? hues[index] : birthHue;
        }
    }
}

void LifeMode::resumeIfChanged() {
    if (!paused) return;
    calculateNext();
    if (memcmp(cells, nextCells, sizeof(cells)) != 0) {
        paused = false;
        lastStep = millis(); // Give a newly spawned pattern one full tick before evolving.
    }
}

bool LifeMode::injectPattern() {
    uint16_t totalWeight = 0;
    for (uint8_t i = 0; i < patternCount; ++i) totalWeight += patterns[i].weight;
    uint16_t pick = random16(totalWeight);
    uint8_t patternIndex = 0;
    for (; patternIndex < patternCount - 1; ++patternIndex) {
        if (pick < patterns[patternIndex].weight) break;
        pick -= patterns[patternIndex].weight;
    }
    const LifePattern& pattern = patterns[patternIndex];
    const uint8_t rotation = random8(4);
    const bool flipped = random8(2) != 0;

    const uint8_t chosenStrip = random8(NUM_STRIPS);
    const uint8_t chosenRow = random8(NUM_LEDS);

    const uint8_t hue = newCellHue(millis());
    bool changed = false;
    for (uint8_t y = 0; y < pattern.height; ++y) {
        for (uint8_t x = 0; x < pattern.width; ++x) {
            if ((pattern.rows[y] & (uint16_t(1) << x)) == 0) continue;
            uint8_t dx, dy;
            patternOffset(pattern, x, y, rotation, flipped, dx, dy);
            const uint8_t strip = (chosenStrip + dx) % NUM_STRIPS;
            const uint8_t row = (chosenRow + dy) % NUM_LEDS;
            if (isAlive(cells, strip, row)) continue;
            setAlive(cells, strip, row);
            changed = true;
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            hues[index] = hue;
            // Pattern spawns appear together even during an ongoing Life fade.
            if (fadeActive) {
                setAlive(previousCells, strip, row);
                previousHues[index] = hue;
            }
        }
    }
    if (!changed) return false;
    renderPending = true;
    resumeIfChanged();
    return true;
}

void LifeMode::loop() {
    const unsigned long now = millis();
    advanceHues(now);
    if (!paused && now - lastStep >= STEP_INTERVAL_MS) {
        lastStep = now;
        calculateNext();
        if (memcmp(cells, nextCells, sizeof(cells)) == 0) {
            paused = true;
            fadeActive = false;
            renderPending = true;
        } else {
            memcpy(previousCells, cells, sizeof(cells));
            memcpy(previousHues, hues, sizeof(hues));
            memcpy(cells, nextCells, sizeof(cells));
            memcpy(hues, nextHues, sizeof(hues));
            fadeActive = true;
            renderPending = true;
        }
    }
    if (fadeActive) renderPending = true;
    bool patternAdded = false;
    if (now - lastPatternInjection >= patternInjectionDelay) {
        patternAdded = injectPattern();
        schedulePatternInjection();
    }
    if (renderPending &&
        (patternAdded || millis() - lastRender >= RENDER_INTERVAL_MS)) {
        lastRender = millis();
        render();
        renderPending = false;
    }
}
