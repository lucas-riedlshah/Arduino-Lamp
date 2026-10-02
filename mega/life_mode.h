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
    static const uint16_t STEP_INTERVAL_MS = 200;
    static const uint16_t RENDER_INTERVAL_MS = 25;
    static const uint8_t CELL_FADE_TICKS = 5; // Same duration for fade in and fade out.
    static const uint32_t CELL_FADE_DURATION_MS = (uint32_t)CELL_FADE_TICKS * STEP_INTERVAL_MS;
    static_assert(CELL_FADE_TICKS > 0 && CELL_FADE_DURATION_MS <= 65535UL,
                  "Cell fade must fit in 16-bit milliseconds");
    static const uint8_t COLOR_TRANSITION_TICKS = 10; // Pink to yellow, independent of fade length.
    static const uint32_t COLOR_TRANSITION_DURATION_MS =
        (uint32_t)COLOR_TRANSITION_TICKS * STEP_INTERVAL_MS;
    static_assert(COLOR_TRANSITION_TICKS > 0 && COLOR_TRANSITION_DURATION_MS <= 65535UL,
                  "Color transition must fit in 16-bit milliseconds");
    static_assert(RENDER_INTERVAL_MS > 0 && STEP_INTERVAL_MS > RENDER_INTERVAL_MS,
                  "Life step must be longer than a positive render interval");
    static const uint8_t PINK_HUE = 220;
    static const uint8_t YELLOW_HUE = 25;
    static_assert(NUM_LEDS > 1, "Background gradient needs at least two LEDs per strip");
    static const bool SHOW_DIM_OFF_CELLS = true; // Set false for black off cells.
    static const uint8_t LIT_CELL_BRIGHTNESS = 255;
    static const uint8_t OFF_CELL_BRIGHTNESS = 100;
    static const unsigned long PATTERN_EMPTY_GRID_INTERVAL_MS = 2500UL;
    static const unsigned long RANDOM_CELL_OFF_INTERVAL_MS = 15000UL;
    static const uint16_t PATTERN_RATE_HALF_LIFE_DOTS = 25;
    static constexpr float PATTERN_INTERVAL_VARIATION = 0.5f; // +/- 50% of the wait.
    static_assert(PATTERN_EMPTY_GRID_INTERVAL_MS > 0, "Pattern interval must be positive");
    static_assert(RANDOM_CELL_OFF_INTERVAL_MS > 0, "Random cell off interval must be positive");
    static_assert(PATTERN_RATE_HALF_LIFE_DOTS > 0, "Pattern rate half-life must be positive");
    static_assert(PATTERN_INTERVAL_VARIATION >= 0.0f && PATTERN_INTERVAL_VARIATION < 1.0f,
                  "Pattern interval variation must be between 0 and 1");

    LifeGrid grid;
    CRGB backgroundColors[NUM_LEDS]; // Gradient by height, shared by all strips.
    uint16_t visualLevels[CELL_COUNT]; // 0 = off colour, CELL_FADE_DURATION_MS = lit colour.
    uint16_t colorAges[CELL_COUNT]; // Time alive, capped at COLOR_TRANSITION_DURATION_MS.
    bool paused = false;
    bool visualTransitionActive = false;
    bool renderPending = false;
    unsigned long lastStep = 0;
    unsigned long lastRender = 0;
    unsigned long lastPatternInjection = 0;
    unsigned long lastRandomCellOff = 0;
    unsigned long lastVisualUpdate = 0;
    unsigned long patternInjectionDelay = 0;

    void clearGrid();
    void buildBackgroundGradient();
    void advanceLife(unsigned long now);
    bool injectIfDue(unsigned long now);
    bool turnRandomCellOffIfDue(unsigned long now);
    void renderIfDue(bool forceRender);
    bool injectPattern();
    void schedulePatternInjection();
    void resumeIfChanged();
    void advanceVisualState(unsigned long now);
    void render();
};

#endif
