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
    {3, 3, gliderRows, 1},
    {5, 4, spaceshipRows, 1},
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

void LifeMode::clearGrid() {
    grid.clear();
    memset(visualLevels, 0, sizeof(visualLevels));
    memset(colorAges, 0, sizeof(colorAges));
    paused = true;
    visualTransitionActive = false;
}

void LifeMode::buildBackgroundGradient() {
    const CHSV yellow(YELLOW_HUE, 255, OFF_CELL_BRIGHTNESS);
    const CHSV pink(PINK_HUE, 255, OFF_CELL_BRIGHTNESS);
    for (uint8_t row = 0; row < NUM_LEDS; ++row) {
        if (SHOW_DIM_OFF_CELLS) {
            float position = (float)row / (NUM_LEDS - 1) *
                BACKGROUND_GRADIENT_STRETCH + backgroundGradientOffset;
            if (position >= 2.0f) position -= 2.0f;
            const float blendPosition = position < 1.0f ? position : 2.0f - position;
            const uint8_t amount = (uint8_t)(blendPosition * 255.0f);
            backgroundColors[row] = blend(yellow, pink, amount);
        } else {
            backgroundColors[row] = CRGB::Black;
        }
    }
}

bool LifeMode::advanceBackgroundIfDue(unsigned long now) {
    if (!SHOW_DIM_OFF_CELLS || now - lastBackgroundMove < backgroundMoveDelay) return false;
    backgroundGradientOffset += BACKGROUND_GRADIENT_STEP;
    if (backgroundGradientOffset >= 2.0f) backgroundGradientOffset -= 2.0f;
    buildBackgroundGradient();
    const long firstWait = random(0, 500);
    const long secondWait = random(0, 500);
    backgroundMoveDelay = firstWait < secondWait ? firstWait : secondWait;
    lastBackgroundMove = millis();
    renderPending = true;
    return true;
}

void LifeMode::prepareNewCell(uint16_t index) {
    if (visualLevels[index] == 0) {
        colorAges[index] = 0;
    } else if (colorAges[index] > COLOR_TRANSITION_DURATION_MS) {
        // A still-visible cell keeps its color and brightness when it relights.
        // Give it the configured post-color lifetime before age expiry.
        colorAges[index] = COLOR_TRANSITION_DURATION_MS;
    }
}

void LifeMode::advanceVisualState(unsigned long now) {
    const unsigned long elapsed = now - lastVisualUpdate;
    lastVisualUpdate = now;
    const uint16_t fadeChange = elapsed >= CELL_FADE_DURATION_MS
        ? CELL_FADE_DURATION_MS : (uint16_t)elapsed;
    const uint16_t ageChange = elapsed >= CELL_AGE_DEATH_THRESHOLD_MS
        ? CELL_AGE_DEATH_THRESHOLD_MS : (uint16_t)elapsed;
    bool transitioning = false;
    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            uint16_t& level = visualLevels[index];
            if (grid.isAlive(strip, row)) {
                level = CELL_FADE_DURATION_MS - level <= fadeChange
                    ? CELL_FADE_DURATION_MS : level + fadeChange;
                if (level < CELL_FADE_DURATION_MS) transitioning = true;
                uint16_t& colorAge = colorAges[index];
                colorAge = CELL_AGE_DEATH_THRESHOLD_MS - colorAge <= ageChange
                    ? CELL_AGE_DEATH_THRESHOLD_MS : colorAge + ageChange;
                if (colorAge < COLOR_TRANSITION_DURATION_MS) transitioning = true;
            } else {
                level = level <= fadeChange ? 0 : level - fadeChange;
                if (level > 0) transitioning = true;
            }
        }
    }
    visualTransitionActive = transitioning;
}

void LifeMode::render() {
    const unsigned long now = millis();
    advanceVisualState(now);
    const CHSV pink(PINK_HUE, 255, 255);
    const CHSV yellow(YELLOW_HUE, 255, 255);
    const CRGB black(0, 0, 0);
    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            const CRGB& offColor = backgroundColors[row];
            const uint16_t level = visualLevels[index];
            if (level > 0) {
                const uint16_t colorAge = colorAges[index] < COLOR_TRANSITION_DURATION_MS
                    ? colorAges[index] : COLOR_TRANSITION_DURATION_MS;
                const uint8_t linearColorAmount =
                    (uint32_t)colorAge * 255 / COLOR_TRANSITION_DURATION_MS;
                const uint8_t colorAmount =
                    (uint16_t)linearColorAmount * linearColorAmount / 255; // Ease in.
                CRGB dotColor(0, 0, 0);
                dotColor = blend(pink, yellow, colorAmount);
                const uint8_t fadeAmount = (uint32_t)level * 255 / CELL_FADE_DURATION_MS;
                if (grid.isAlive(strip, row)) {
                    const uint8_t dotBrightness =
                        (uint32_t)level * LIT_CELL_BRIGHTNESS / CELL_FADE_DURATION_MS;
                    const uint8_t backgroundBrightness =
                        SHOW_DIM_OFF_CELLS ? OFF_CELL_BRIGHTNESS : 0;
                    if (backgroundBrightness > 0 && dotBrightness < backgroundBrightness) {
                        // Crossfade only until the dot matches the background's brightness.
                        const CRGB matchingDotColor =
                            blend(black, dotColor, backgroundBrightness);
                        const uint8_t backgroundMix =
                            (uint16_t)dotBrightness * 255 / backgroundBrightness;
                        leds[index] = blend(offColor, matchingDotColor, backgroundMix);
                    } else {
                        leds[index] = blend(black, dotColor, dotBrightness);
                    }
                } else {
                    const CRGB litColor = blend(black, dotColor, LIT_CELL_BRIGHTNESS);
                    leds[index] = blend(offColor, litColor, fadeAmount);
                }
            } else {
                leds[index] = offColor;
            }
        }
    }
    FastLED.show();
}

void LifeMode::schedulePatternInjection() {
    lastPatternInjection = millis();
    // The wait doubles smoothly for every PATTERN_RATE_HALF_LIFE_DOTS lit cells.
    const float waitMs = PATTERN_EMPTY_GRID_INTERVAL_MS *
        expf(0.69314718f * grid.liveCount() / PATTERN_RATE_HALF_LIFE_DOTS);
    // Keep the random() bounds within signed 32-bit milliseconds.
    const float maxMeanMs = 2000000000.0f / (1.0f + PATTERN_INTERVAL_VARIATION);
    const float boundedWaitMs = waitMs < maxMeanMs ? waitMs : maxMeanMs;
    const long earliest = (long)(boundedWaitMs * (1.0f - PATTERN_INTERVAL_VARIATION) + 0.5f);
    const long latest = (long)(boundedWaitMs * (1.0f + PATTERN_INTERVAL_VARIATION) + 0.5f);
    patternInjectionDelay = random(earliest, latest + 1);
}

void LifeMode::setup() {
    lastVisualUpdate = millis();
    clearGrid();
    backgroundGradientOffset = 0.0f;
    lastBackgroundMove = lastVisualUpdate;
    backgroundMoveDelay = 0;
    buildBackgroundGradient();
    render();
    lastRender = millis();
    renderPending = false;
    lastStep = lastRender;
    schedulePatternInjection();
}

void LifeMode::resumeIfChanged() {
    if (!paused) return;
    grid.calculateNext();
    if (grid.hasChanged()) {
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

    advanceVisualState(millis());
    bool changed = false;
    for (uint8_t y = 0; y < pattern.height; ++y) {
        for (uint8_t x = 0; x < pattern.width; ++x) {
            if ((pattern.rows[y] & (uint16_t(1) << x)) == 0) continue;
            uint8_t dx, dy;
            patternOffset(pattern, x, y, rotation, flipped, dx, dy);
            const uint8_t strip = (chosenStrip + dx) % NUM_STRIPS;
            const uint8_t row = (chosenRow + dy) % NUM_LEDS;
            if (grid.isAlive(strip, row)) continue;
            grid.setAlive(strip, row);
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            prepareNewCell(index);
            changed = true;
        }
    }
    renderPending = true;
    if (!changed) return false;
    visualTransitionActive = true;
    resumeIfChanged();
    return true;
}

void LifeMode::advanceLife(unsigned long now) {
    if (now - lastStep >= STEP_INTERVAL_MS) {
        advanceVisualState(now);
        lastStep = now;
        if (!paused) {
            grid.calculateNext();
            if (!grid.hasChanged()) {
                paused = true;
                renderPending = true;
            } else {
                for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
                    for (uint8_t row = 0; row < NUM_LEDS; ++row) {
                        if (grid.nextIsAlive(strip, row) && !grid.isAlive(strip, row)) {
                            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
                            prepareNewCell(index);
                        }
                    }
                }
                grid.commit();
                visualTransitionActive = true;
                renderPending = true;
            }
        }
        killOldCells();
    }
}

bool LifeMode::injectIfDue(unsigned long now) {
    bool patternAdded = false;
    if (now - lastPatternInjection >= patternInjectionDelay) {
        patternAdded = injectPattern();
        schedulePatternInjection();
    }
    return patternAdded;
}

void LifeMode::killOldCells() {
    for (uint8_t strip = 0; strip < NUM_STRIPS; ++strip) {
        for (uint8_t row = 0; row < NUM_LEDS; ++row) {
            if (!grid.isAlive(strip, row)) continue;
            const uint16_t index = (uint16_t)strip * NUM_LEDS + row;
            if (colorAges[index] < CELL_AGE_DEATH_THRESHOLD_MS) continue;

            bool hasYoungerLiveNeighbor = false;
            for (int8_t dx = -1; dx <= 1 && !hasYoungerLiveNeighbor; ++dx) {
                int8_t neighborStrip = (int8_t)strip + dx;
                if (neighborStrip < 0) neighborStrip = NUM_STRIPS - 1;
                if (neighborStrip >= NUM_STRIPS) neighborStrip = 0;
                for (int8_t dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    int8_t neighborRow = (int8_t)row + dy;
                    if (neighborRow < 0) neighborRow = NUM_LEDS - 1;
                    if (neighborRow >= NUM_LEDS) neighborRow = 0;

                    const uint16_t neighborIndex =
                        (uint16_t)neighborStrip * NUM_LEDS + neighborRow;
                    if (grid.isAlive(neighborStrip, neighborRow) &&
                        colorAges[neighborIndex] < CELL_AGE_DEATH_THRESHOLD_MS) {
                        hasYoungerLiveNeighbor = true;
                        break;
                    }
                }
            }
            if (!hasYoungerLiveNeighbor) continue;

            grid.setDead(strip, row);
            paused = false; // Resume Conway on the next tick after a stable grid changes.
            visualTransitionActive = true;
            renderPending = true;
        }
    }
}

void LifeMode::renderIfDue(bool forceRender) {
    if (renderPending && (forceRender || millis() - lastRender >= RENDER_INTERVAL_MS)) {
        lastRender = millis();
        render();
        renderPending = false;
    }
}

void LifeMode::loop() {
    const unsigned long now = millis();
    advanceLife(now);
    const bool patternAdded = injectIfDue(now);
    const bool backgroundMoved = advanceBackgroundIfDue(now);
    if (visualTransitionActive) renderPending = true;
    renderIfDue(patternAdded || backgroundMoved);
}
