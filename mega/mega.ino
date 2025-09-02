#include "FastLED.h" // version 3.9.20
#include <EEPROM.h>
#include "shared_config.h"
#include "candle_mode.h"
#include "noise_mode.h"
#include "gradient_mode.h"
#include "ModeInterface.h"

#define MODE_ADDR 0   // EEPROM address to store mode (0=candle, 1=noise, 2=gradient, 3=moving gradient)

CRGB leds[NUM_LEDS * NUM_STRIPS];

// Mode variable
uint8_t mode = 0;
ModeInterface* currentMode = nullptr;

void switchMode(uint8_t newMode) {
    if (currentMode) {
        currentMode->cleanup();
        delete currentMode;
        currentMode = nullptr;
    }
    mode = newMode;
    switch (mode) {
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
        default:
            // Moving gradient: yellow to pink
            currentMode = new GradientMode(CHSV(43, 255, 255), CHSV(220, 255, 255), true);
            break;
    }
    if (currentMode) currentMode->setup();
}

void setup() {
    Serial.begin(9600);
    FastLED.addLeds< WS2812B, DATA_START + 0, GRB >(leds, 0 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 11, GRB >(leds, 1 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 3, GRB >(leds, 2 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 5, GRB >(leds, 3 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 8, GRB >(leds, 4 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 4, GRB >(leds, 5 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 10, GRB >(leds, 6 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 1, GRB >(leds, 7 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 2, GRB >(leds, 8 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 6, GRB >(leds, 9 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 7, GRB >(leds, 10 * NUM_LEDS, NUM_LEDS);
    FastLED.addLeds< WS2812B, DATA_START + 9, GRB >(leds, 11 * NUM_LEDS, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 19000);
    randomSeed(analogRead(8));
    FastLED.setBrightness(0);
    fill_solid(leds, NUM_LEDS * NUM_STRIPS, CRGB::Black);
    FastLED.show();
    FastLED.setBrightness(100);
    // ---- EEPROM logic ----
    mode = EEPROM.read(MODE_ADDR);      // read saved mode
    mode = (mode + 1) % 4;              // cycle between 0, 1, 2, 3
    EEPROM.write(MODE_ADDR, mode);      // save new mode
    Serial.print("Mode selected: ");
    Serial.println(mode);
    switchMode(mode);
}

void loop() {
    if (currentMode) currentMode->loop();
}
