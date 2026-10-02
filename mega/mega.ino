#include "FastLED.h" // version 3.9.20
#include <EEPROM.h>
#include "shared_config.h"
#include "candle_mode.h"
#include "noise_mode.h"
#include "gradient_mode.h"
#include "bounce_mode.h"
#include "gradient_candle_mode.h"
#include "life_mode.h"
#include "strip_test_mode.h"
#include "ModeInterface.h"

#define MODE_ADDR 0
#define MODE_COUNT 7
#define TEMP_STRIP_TEST 0 // Set to 1 to show the strip mapping test.

CRGB leds[NUM_LEDS * NUM_STRIPS];

// Mode variable
uint8_t mode = 0;
ModeInterface* currentMode = nullptr;

// FastLED requires the data pin as a compile-time template argument. This
// recursion registers the strips in the same order as stripPinsByPosition.
template <uint8_t Position>
struct RegisterStrip {
    static void run() {
        FastLED.addLeds<WS2812B, stripPinsByPosition[Position], GRB>(
            leds, (uint16_t)Position * NUM_LEDS, NUM_LEDS);
        RegisterStrip<Position + 1>::run();
    }
};

template <>
struct RegisterStrip<NUM_STRIPS> {
    static void run() {}
};

void switchMode(uint8_t newMode) {
    if (currentMode) {
        currentMode->cleanup();
        delete currentMode;
        currentMode = nullptr;
    }
    mode = newMode;
    switch (TEMP_STRIP_TEST ? 7 : 6) {
        case 0:
            currentMode = new CandleMode();
            break;
        case 1:
            currentMode = new NoiseMode();
            break;
        case 2:
            // Static gradient: yellow to pink
            currentMode = new GradientMode(CHSV(43, 255, 255), CHSV(220, 255, 255), false);
            break;
        case 3: {
            // Example: parse colors (could be from config, user input, etc.)
            CRGB ringColor = CRGB(0, 0, 255);
            CRGB fadeColor = CRGB(55, 0, 255);
            currentMode = new BounceMode(ringColor, fadeColor);
            break;
        }
        case 4: {
            currentMode = new GradientCandleMode();
            break;
        }
        case 5:
            // Moving gradient: yellow to pink
            currentMode = new GradientMode(CHSV(43, 255, 255), CHSV(220, 255, 255), true);
            break;
        case 6:
            currentMode = new LifeMode();
            break;
        case 7:
            currentMode = new StripTestMode();
            break;
    }
    if (currentMode) currentMode->setup();
}

void setup() {
    Serial.begin(9600);
    RegisterStrip<0>::run();
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 19000);
    randomSeed(analogRead(8));
    FastLED.setBrightness(0);
    fill_solid(leds, NUM_LEDS * NUM_STRIPS, CRGB::Black);
    FastLED.show();
    FastLED.setBrightness(255);
    // ---- EEPROM logic ----
    mode = EEPROM.read(MODE_ADDR);      // read saved mode
    mode = (mode + 1) % MODE_COUNT;
    EEPROM.write(MODE_ADDR, mode);      // save new mode
    Serial.print("Mode selected: ");
    Serial.println(mode);
    switchMode(mode);
}

void loop() {
    if (currentMode) currentMode->loop();
}
