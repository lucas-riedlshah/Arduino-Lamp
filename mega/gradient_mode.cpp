#include "gradient_mode.h"
#include "shared_config.h"

// Moving gradient variables
float gradientOffset = 0.0;
const float gradientSpeed = 0.02; // Speed of gradient movement

void paintGradient(CHSV colorStart, CHSV colorEnd) {
  // Vertical gradient: blend by LED index within each strip
  for (int strip = 0; strip < NUM_STRIPS; strip++) {
    for (int led = 0; led < NUM_LEDS; led++) {
      uint8_t blendAmount = map(led, 0, NUM_LEDS - 1, 0, 255);
      leds[strip * NUM_LEDS + led] = blend(colorStart, colorEnd, blendAmount);
    }
  }
  FastLED.show();
  delay(30);
}

void paintMovingGradient(CHSV colorStart, CHSV colorEnd) {
  // Update gradient offset for bidirectional fade
  gradientOffset += gradientSpeed;
  if (gradientOffset >= 2.0) {
    gradientOffset -= 2.0; // Loop back to 0 for full cycle
  }
  
  // Create bidirectional fade: A->B->A->B...
  for (int strip = 0; strip < NUM_STRIPS; strip++) {
    for (int led = 0; led < NUM_LEDS; led++) {
      // Calculate position with offset for movement
      float ledPos = (float)led / (NUM_LEDS - 1) + gradientOffset;
      if (ledPos >= 2.0) {
        ledPos -= 2.0; // Wrap around for full cycle
      }
      
      // Create bidirectional fade effect with asymmetric timing
      float fadePos;
      if (ledPos <= 1.0) {
        // First half: fade from A to B (normal speed)
        fadePos = ledPos;
      } else {
        // Second half: fade from B back to A (4x faster)
        // Compress the 1.0-2.0 range into 0.0-1.0 range, then scale by 4
        float compressedPos = (ledPos - 1.0) * 4.0;
        fadePos = 1.0 - compressedPos; // Invert for B->A direction
        if (fadePos < 0.0) fadePos = 0.0; // Clamp to prevent negative values
      }
      
      // Apply smooth curve for softer transitions
      fadePos = fadePos * fadePos * (3.0 - 2.0 * fadePos); // Smoothstep function
      
      // Convert to blend amount
      uint8_t blendAmount = (uint8_t)(fadePos * 255);
      leds[strip * NUM_LEDS + led] = blend(colorStart, colorEnd, blendAmount);
    }
  }
  FastLED.show();
  delay(min(random(0, 500), random(0, 500)));
}
