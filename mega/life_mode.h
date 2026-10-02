#ifndef LIFE_MODE_H
#define LIFE_MODE_H

#include "ModeInterface.h"
#include "shared_config.h"
#include "life_grid.h"

class LifeMode : public ModeInterface {
public:
    void setup() override;
    void loop() override;

private:
    static const uint16_t CELL_COUNT = LifeGrid::CELL_COUNT;
    static constexpr float SPEED_MULTIPLIER = 1.0f; // 0.5 = twice as slow; 2 = twice as fast.
    static_assert(SPEED_MULTIPLIER >= 0.1f, "Speed multiplier must be at least 0.1");
    static const uint16_t BASE_STEP_INTERVAL_MS = 200;
    static const uint16_t BASE_RENDER_INTERVAL_MS = 25;
    static const unsigned long BASE_PATTERN_EMPTY_GRID_INTERVAL_MS = 2500UL;
    static const uint16_t STEP_INTERVAL_MS =
        (uint16_t)(BASE_STEP_INTERVAL_MS / SPEED_MULTIPLIER + 0.5f);
    static const uint16_t RENDER_INTERVAL_MS =
        (uint16_t)(BASE_RENDER_INTERVAL_MS / SPEED_MULTIPLIER + 0.5f);
    static const uint8_t CELL_FADE_TICKS = 5; // Same duration for fade in and fade out.
    static const uint32_t CELL_FADE_DURATION_MS = (uint32_t)CELL_FADE_TICKS * STEP_INTERVAL_MS;
    static_assert(CELL_FADE_TICKS > 0 && CELL_FADE_DURATION_MS <= 65535UL,
                  "Cell fade must fit in 16-bit milliseconds");
    static_assert(RENDER_INTERVAL_MS > 0 && STEP_INTERVAL_MS > RENDER_INTERVAL_MS,
                  "Life step must be longer than a positive render interval");
    static const uint8_t PURPLE_HUE = 200;
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

    LifeGrid grid;
    uint16_t visualLevels[CELL_COUNT]; // 0 = off colour, CELL_FADE_DURATION_MS = lit colour.
    bool paused = false;
    bool visualTransitionActive = false;
    bool renderPending = false;
    unsigned long lastStep = 0;
    unsigned long lastRender = 0;
    unsigned long lastPatternInjection = 0;
    unsigned long lastVisualUpdate = 0;
    unsigned long patternInjectionDelay = 0;

    void clearGrid();
    void advanceLife(unsigned long now);
    bool injectIfDue(unsigned long now);
    void renderIfDue(bool patternAdded);
    bool injectPattern();
    void schedulePatternInjection();
    void resumeIfChanged();
    void advanceVisualLevels(unsigned long now);
    void render();
};

#endif
