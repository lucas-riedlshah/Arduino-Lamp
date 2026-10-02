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
    static const uint16_t STEP_INTERVAL_MS = 500;
    static const uint16_t RENDER_INTERVAL_MS = 25;
    static const uint8_t CELL_FADE_TICKS = 5; // Same duration for fade in and fade out.
    static const uint32_t CELL_FADE_DURATION_MS = (uint32_t)CELL_FADE_TICKS * STEP_INTERVAL_MS;
    static const uint8_t COLOR_TRANSITION_TICKS = 10; // Pink to yellow.
    static const uint32_t COLOR_TRANSITION_DURATION_MS =
        (uint32_t)COLOR_TRANSITION_TICKS * STEP_INTERVAL_MS;
    static const uint16_t CELL_POST_COLOR_LIFETIME_MS = 1000;
    static const uint16_t CELL_AGE_DEATH_THRESHOLD_MS =
        (uint32_t)COLOR_TRANSITION_DURATION_MS + CELL_POST_COLOR_LIFETIME_MS;
    static const uint8_t PINK_HUE = 220;
    static const uint8_t YELLOW_HUE = 20;
    static constexpr float BACKGROUND_GRADIENT_STRETCH = 0.5f;
    static constexpr float BACKGROUND_GRADIENT_STEP = 0.02f;
    static const bool SHOW_DIM_OFF_CELLS = false; // Set false for black off cells.
    static const uint8_t LIT_CELL_BRIGHTNESS = 255;
    static const uint8_t OFF_CELL_BRIGHTNESS = 100;
    static const unsigned long PATTERN_EMPTY_GRID_INTERVAL_MS = 2000UL;
    static const uint16_t PATTERN_RATE_HALF_LIFE_DOTS = 200;
    static constexpr float PATTERN_INTERVAL_VARIATION = 1.0f;

    static_assert(CELL_FADE_TICKS > 0 && CELL_FADE_DURATION_MS <= 65535UL,
                  "Cell fade must fit in 16-bit milliseconds");
    static_assert(COLOR_TRANSITION_TICKS > 0 && COLOR_TRANSITION_DURATION_MS <= 65535UL,
                  "Color transition must fit in 16-bit milliseconds");
    static_assert(COLOR_TRANSITION_DURATION_MS + CELL_POST_COLOR_LIFETIME_MS <= 65535UL,
                  "Cell lifetime must fit in 16-bit milliseconds");
    static_assert(RENDER_INTERVAL_MS > 0 && STEP_INTERVAL_MS > RENDER_INTERVAL_MS,
                  "Life step must be longer than a positive render interval");
    static_assert(NUM_LEDS > 1, "Background gradient needs at least two LEDs per strip");
    static_assert(PATTERN_EMPTY_GRID_INTERVAL_MS > 0, "Pattern interval must be positive");
    static_assert(PATTERN_RATE_HALF_LIFE_DOTS > 0, "Pattern rate half-life must be positive");
    static_assert(PATTERN_INTERVAL_VARIATION >= 0.0f && PATTERN_INTERVAL_VARIATION <= 1.0f,
                  "Pattern interval variation must be between 0 and 1");

    LifeGrid grid;
    CRGB backgroundColors[NUM_LEDS]; // Gradient by height, shared by all strips.
    uint16_t visualLevels[CELL_COUNT]; // 0 = off colour, CELL_FADE_DURATION_MS = lit colour.
    uint16_t colorAges[CELL_COUNT]; // Visual color age and expiry age, capped at CELL_AGE_DEATH_THRESHOLD_MS.
    bool paused = false;
    bool visualTransitionActive = false;
    bool renderPending = false;
    unsigned long lastStep = 0;
    unsigned long lastRender = 0;
    unsigned long lastPatternInjection = 0;
    unsigned long lastVisualUpdate = 0;
    unsigned long lastBackgroundMove = 0;
    unsigned long patternInjectionDelay = 0;
    unsigned long backgroundMoveDelay = 0;
    float backgroundGradientOffset = 0.0f;

    void clearGrid();
    void buildBackgroundGradient();
    bool advanceBackgroundIfDue(unsigned long now);
    void advanceLife(unsigned long now);
    bool injectIfDue(unsigned long now);
    void killOldCells();
    void renderIfDue(bool forceRender);
    bool injectPattern();
    void schedulePatternInjection();
    void resumeIfChanged();
    void prepareNewCell(uint16_t index);
    void advanceVisualState(unsigned long now);
    void render();
};

#endif
