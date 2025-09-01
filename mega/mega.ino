#include "FastLED.h" // version 3.9.20

#define NUM_LEDS 48
#define NUM_STRIPS 12
#define DATA_START 2
#define SPEED 20
#define SCALE 5
#define STEPS 5

CRGB leds[NUM_LEDS * NUM_STRIPS];

CRGB noise[2][NUM_STRIPS][NUM_LEDS];
//uint8_t red_noise[2][NUM_STRIPS][NUM_LEDS];
//uint8_t green_noise[2][NUM_STRIPS][NUM_LEDS];
//uint8_t blue_noise[2][NUM_STRIPS][NUM_LEDS];

uint8_t remap[256] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 23, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84, 85, 86, 88, 89, 90, 91, 92, 94, 95, 96, 97, 98, 100, 101, 102, 103, 105, 106, 107, 109, 110, 111, 113, 114, 115, 117, 118, 119, 121, 122, 123, 125, 126, 128, 129, 130, 132, 133, 135, 136, 138, 139, 141, 142, 144, 145, 147, 148, 150, 151, 153, 154, 156, 157, 159, 161, 162, 164, 165, 167, 169, 170, 172, 173, 175, 177, 178, 180, 182, 183, 185, 187, 189, 190, 192, 194, 196, 197, 199, 201, 203, 204, 206, 208, 210, 212, 213, 215, 217, 219, 221, 223, 225, 226, 228, 230, 232, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254, 255 };

static uint16_t roff;
static uint16_t goff;
static uint16_t boff;
static uint16_t z;

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

  z = random16();
  roff = random16();
  goff = random16();
  boff = random16();

  FastLED.setBrightness(0);
  fill_solid(leds, NUM_LEDS * NUM_STRIPS, CRGB::Black);
  FastLED.show();

  FastLED.setBrightness(100);

  fillnoise8();
}

void loop() {
  paintCandleFlicker(CHSV(40, 255, 190), CHSV(40, 150, 255));
//  paintCandleFlicker(CHSV(40, 255, 0), CHSV(40, 255, 255)); // DEBUG COLOR RANGE
  // paintNoise();
}

uint8_t steady, flicker, turb;
uint8_t v;
CHSV color;
int f = 0;
int g;
#define CANDLE_STEADY_FREQ 0.1
#define CANDLE_FLICKER_FREQ 2
#define CANDLE_TURB_FREQ 0.1
#define CANDLE_FLICKER_THRESHOLD 80 // 0-255, 80 means more turbulence - flickers more often
void paintCandleFlicker(CHSV color1, CHSV color2) {
  steady = mynoise(10000, millis() * CANDLE_STEADY_FREQ);
  flicker = mynoise(20000, millis() * CANDLE_FLICKER_FREQ);
  turb = mynoise(30000, millis() * CANDLE_TURB_FREQ);

  v = (turb < CANDLE_FLICKER_THRESHOLD ? 0.6 : 1) * steady + (turb < CANDLE_FLICKER_THRESHOLD ? 0.4 : 0) * flicker;

//  Serial.println(v);
  
  color = blend(color1, color2, v);

  f += 1;
  g = f & 1;
  for (int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
    if ((i & 1) == g) {
      leds[i] = color;
    }
//    if (random(0, 100) / 100.0 < CANDLE_UPDATE_PERCENTAGE) {
//      leds[i] = color;
//    }
  }
  FastLED.show();
}

void paintSolidColour(CHSV color) {
  for (int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
    // leds[i] = CHSV(int(random(0, 255)), 255, 255);
    leds[i] = color;
  }
  FastLED.show();
  delay(100);
}

void paintNoise() {
  fillnoise8();

  for (int p = 0; p < STEPS; p++) {
    for (int i = 0; i < NUM_STRIPS; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        //        CRGB c = blend(CRGB(red_noise[0][i][j], green_noise[0][i][j], blue_noise[0][i][j]), CRGB(red_noise[1][i][j], green_noise[1][i][j], blue_noise[1][i][j]), p * 255 / STEPS);
        leds[i * 48 + j] = blend(noise[0][i][j], noise[1][i][j], p * 255 / STEPS);
      }
    }

    FastLED.show();
    delay(3);
  }
}

void fillnoise8() {
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      noise[0][i][j] = noise[1][i][j];
      //      red_noise[0][i][j] = red_noise[1][i][j];
      //      green_noise[0][i][j] = green_noise[1][i][j];
      //      blue_noise[0][i][j] = blue_noise[1][i][j];

      int joffset = j * SCALE * 2 + z * 0.1;
      float x = sin(i * PI / 6) * SCALE + z;
      float y = cos(i * PI / 6) * SCALE + z;
      noise[1][i][j] = CRGB(remap[inoise8(roff + x, roff + y, joffset)], remap[inoise8(goff + x, goff + y, joffset)], remap[inoise8(boff + x, boff + y, joffset)]);
      //      red_noise[1][i][j] = remap[inoise8(roff + x, roff + y, joffset)];
      //      blue_noise[1][i][j] = remap[inoise8(goff + x, goff + y, joffset)];
      //      green_noise[1][i][j] = remap[inoise8(boff + x, boff + y, joffset)];
    }
  }
  z += SPEED; // TODO: Need to fix this to use millis() so that it doesn't change with framerate 
}

uint8_t mynoise(uint16_t x, uint16_t y) {
//  return inoise16(x << 8, y << 8) >> 8;
  return inoise8(x, y);
}
