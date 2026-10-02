#ifndef LIFE_MODE_H
#define LIFE_MODE_H

#include "ModeInterface.h"
#include "shared_config.h"

class LifeMode : public ModeInterface {
public:
    void setup() override;
    void loop() override;

private:
    static const uint16_t CELL_COUNT = NUM_STRIPS * NUM_LEDS;
    static const uint16_t STATE_BYTES = (CELL_COUNT + 7) / 8;
    static constexpr float SPEED_MULTIPLIER = 1.0f; // 0.5 = twice as slow; 2 = twice as fast.
    static_assert(SPEED_MULTIPLIER >= 0.1f, "Speed multiplier must be at least 0.1");
    static const uint16_t BASE_STEP_INTERVAL_MS = 200;
    static const uint16_t BASE_RENDER_INTERVAL_MS = 25;
    static const unsigned long BASE_HUE_CYCLE_MS = 25000UL;
    static const unsigned long BASE_NEW_CELL_HUE_CYCLE_MS = 1500000UL;
    static const unsigned long BASE_PATTERN_EMPTY_GRID_INTERVAL_MS = 2500UL;
    static const uint16_t STEP_INTERVAL_MS =
        (uint16_t)(BASE_STEP_INTERVAL_MS / SPEED_MULTIPLIER + 0.5f);
    static const uint16_t RENDER_INTERVAL_MS =
        (uint16_t)(BASE_RENDER_INTERVAL_MS / SPEED_MULTIPLIER + 0.5f);
    static const uint16_t FADE_DURATION_MS = STEP_INTERVAL_MS;
    static_assert(RENDER_INTERVAL_MS > 0 && STEP_INTERVAL_MS > RENDER_INTERVAL_MS,
                  "Life step must be longer than a positive render interval");
    static const uint8_t PURPLE_HUE = 200;
    static const unsigned long HUE_CYCLE_MS =
        (unsigned long)(BASE_HUE_CYCLE_MS / SPEED_MULTIPLIER + 0.5f);
    static const unsigned long NEW_CELL_HUE_CYCLE_MS =
        (unsigned long)(BASE_NEW_CELL_HUE_CYCLE_MS / SPEED_MULTIPLIER + 0.5f);
    static_assert(HUE_CYCLE_MS > 0 && NEW_CELL_HUE_CYCLE_MS > 0,
                  "Hue cycles must be positive");
    static_assert(NEW_CELL_HUE_CYCLE_MS <= 16000000UL,
                  "New-cell hue cycle is too long for 32-bit hue math");
    static const bool SHOW_DIM_OFF_CELLS = true; // Set false for black off cells.
    static const uint8_t LIT_CELL_BRIGHTNESS = 255;
    static const uint8_t OFF_CELL_BRIGHTNESS = 100;
    // Whole-pattern wait doubles every 25 live cells.
    static const unsigned long PATTERN_EMPTY_GRID_INTERVAL_MS =
        (unsigned long)(BASE_PATTERN_EMPTY_GRID_INTERVAL_MS / SPEED_MULTIPLIER + 0.5f);
    static const uint16_t PATTERN_RATE_HALF_LIFE_DOTS = 25;
    static constexpr float PATTERN_INTERVAL_VARIATION = 0.5f; // +/- 50% of the wait.
    static_assert(PATTERN_EMPTY_GRID_INTERVAL_MS > 0, "Pattern interval must be positive");
    static_assert(PATTERN_RATE_HALF_LIFE_DOTS > 0, "Pattern rate half-life must be positive");
    static_assert(PATTERN_INTERVAL_VARIATION >= 0.0f && PATTERN_INTERVAL_VARIATION < 1.0f,
                  "Pattern interval variation must be between 0 and 1");

    uint8_t cells[STATE_BYTES];
    uint8_t nextCells[STATE_BYTES];
    uint8_t previousCells[STATE_BYTES];
    uint8_t hues[CELL_COUNT];
    uint8_t nextHues[CELL_COUNT];
    uint8_t previousHues[CELL_COUNT];
    bool paused = false;
    bool fadeActive = false;
    bool renderPending = false;
    unsigned long lastStep = 0;
    unsigned long lastRender = 0;
    unsigned long lastPatternInjection = 0;
    unsigned long lastHueUpdate = 0;
    unsigned long modeStartedAt = 0;
    uint8_t lastOffHue = PURPLE_HUE;
    unsigned long hueRemainder = 0;
    unsigned long patternInjectionDelay = 0;

    static bool isAlive(const uint8_t* state, uint8_t strip, uint8_t row);
    static void setAlive(uint8_t* state, uint8_t strip, uint8_t row);
    uint8_t countNeighbors(uint8_t strip, uint8_t row) const;
    void clearGrid();
    void calculateNext();
    bool injectPattern();
    void schedulePatternInjection();
    void resumeIfChanged();
    uint16_t liveCellCount() const;
    void advanceHues(unsigned long now);
    uint8_t newCellHue(unsigned long now) const;
    void render();
};

#endif
