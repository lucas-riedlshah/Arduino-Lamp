#include "noise_mode.h"
#include "shared_config.h"

// remap table
uint8_t remap[256] = { 
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,4,4,4,4,
  5,5,5,5,6,6,6,6,7,7,7,8,8,8,9,9,10,10,10,11,11,12,12,12,13,13,14,14,
  15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,23,23,24,24,25,26,26,27,
  28,28,29,30,30,31,32,32,33,34,34,35,36,37,37,38,39,40,41,41,42,43,44,
  45,45,46,47,48,49,50,51,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,
  66,67,68,69,70,71,72,73,74,75,76,77,78,80,81,82,83,84,85,86,88,89,90,
  91,92,94,95,96,97,98,100,101,102,103,105,106,107,109,110,111,113,114,
  115,117,118,119,121,122,123,125,126,128,129,130,132,133,135,136,138,
  139,141,142,144,145,147,148,150,151,153,154,156,157,159,161,162,164,
  165,167,169,170,172,173,175,177,178,180,182,183,185,187,189,190,192,
  194,196,197,199,201,203,204,206,208,210,212,213,215,217,219,221,223,
  225,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,255 
};


void NoiseMode::setup() {
    // Allocate noise array
    noise = new CRGB**[2];
    for (int i = 0; i < 2; i++) {
        noise[i] = new CRGB*[NUM_STRIPS];
        for (int j = 0; j < NUM_STRIPS; j++) {
            noise[i][j] = new CRGB[NUM_LEDS];
        }
    }
    z = random16();
    roff = random16();
    goff = random16();
    boff = random16();
}

void NoiseMode::loop() {
    // fillnoise8
    for (int i = 0; i < NUM_STRIPS; i++) {
        for (int j = 0; j < NUM_LEDS; j++) {
            noise[0][i][j] = noise[1][i][j];
            int joffset = j * SCALE * 2 + z * 0.1;
            float x = sin(i * PI / 6) * SCALE + z;
            float y = cos(i * PI / 6) * SCALE + z;
            noise[1][i][j] = CRGB(
                remap[inoise8(roff + x, roff + y, joffset)],
                remap[inoise8(goff + x, goff + y, joffset)],
                remap[inoise8(boff + x, boff + y, joffset)]
            );
        }
    }
    z++;

    for (int p = 0; p < STEPS; p++) {
        for (int i = 0; i < NUM_STRIPS; i++) {
            for (int j = 0; j < NUM_LEDS; j++) {
                leds[i * 48 + j] = blend(noise[0][i][j], noise[1][i][j], p * 255 / STEPS);
            }
        }
        FastLED.show();
        delay(3);
    }
}

void NoiseMode::cleanup() {
    if (noise) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < NUM_STRIPS; j++) {
                delete[] noise[i][j];
            }
            delete[] noise[i];
        }
        delete[] noise;
        noise = nullptr;
    }
}
