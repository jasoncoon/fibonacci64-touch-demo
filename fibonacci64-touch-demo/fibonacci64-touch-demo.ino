/*
   Fibonacci64 Touch Demo: https://github.com/jasoncoon/fibonacci64-touch-demo
   Copyright (C) 2021 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <FastLED.h>  // https://github.com/FastLED/FastLED
#include "Adafruit_FreeTouch.h" //https://github.com/adafruit/Adafruit_FreeTouch
#include "GradientPalettes.h"

FASTLED_USING_NAMESPACE

#define DATA_PIN      A10
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define NUM_LEDS      182

#define MILLI_AMPS         1400 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND  1000

CRGB leds[NUM_LEDS];

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];

uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette( CRGB::Black);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
uint8_t secondsPerPalette = 10;

#include "Map.h"

uint8_t brightness = 32;

Adafruit_FreeTouch touch0 = Adafruit_FreeTouch(A0, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch1 = Adafruit_FreeTouch(A1, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch2 = Adafruit_FreeTouch(A2, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch3 = Adafruit_FreeTouch(A3, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch4 = Adafruit_FreeTouch(A6, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch5 = Adafruit_FreeTouch(A7, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);

#define touchPointCount 6

// These values were discovered using the commented-out Serial.print statements in handleTouch below

// minimum values for each touch pad, used to filter out noise
uint16_t touchMin[touchPointCount] = { 558, 259, 418, 368, 368, 368 };

// maximum values for each touch pad, used to determine when a pad is touched
uint16_t touchMax[touchPointCount] = { 1016, 1016, 1016, 1016, 1016, 1016 };

// raw capacitive touch sensor readings
uint16_t touchRaw[touchPointCount] = { 0, 0, 0, 0, 0, 0 };

// capacitive touch sensor readings, mapped/scaled one one byte each (0-255)
uint8_t touch[touchPointCount] = { 0, 0, 0, 0, 0, 0 };

// coordinates of the touch points
uint8_t touchPointX[touchPointCount] = { 255, 127,   0,   0, 127, 255 };
uint8_t touchPointY[touchPointCount] = {   0,   0,   0, 255, 255, 255 };

uint8_t touchEnabled[touchPointCount] = { false, false, false, false, false, false };

boolean activeWaves = false;

uint8_t offset = 0;

#include "vector.h"
#include "boid.h"
#include "attractor.h"
#include "attract.h"

void setup() {
  Serial.begin(115200);
  //  delay(3000);

  touchEnabled[0] = touch0.begin();
  touchEnabled[1] = touch1.begin();
  touchEnabled[2] = touch2.begin();
  touchEnabled[3] = touch3.begin();
  touchEnabled[4] = touch4.begin();
  touchEnabled[5] = touch5.begin();

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalSMD5050);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  FastLED.setBrightness(brightness);

  startAttract();
}

void loop() {
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random());

  handleTouch();

  // change to a new cpt-city gradient palette
  EVERY_N_SECONDS( secondsPerPalette ) {
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
  }

  EVERY_N_MILLISECONDS(30) {
    // slowly blend the current palette to the next
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 8);
    offset++;
  }

  if (!activeWaves)
    attract();
    // colorWavesFibonacci();

  // touchDemo();

  // FastLED.show();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

bool touchChanged = true;

void handleTouch() {
  for (uint8_t i = 0; i < touchPointCount; i++) {
    if (i == 0) touchRaw[i] = touch0.measure();
    else if (i == 1) touchRaw[i] = touch1.measure();
    else if (i == 2) touchRaw[i] = touch2.measure();
    else if (i == 3) touchRaw[i] = touch3.measure();
    else if (i == 4) touchRaw[i] = touch4.measure();
    else if (i == 5) touchRaw[i] = touch5.measure();

    // // uncomment to display raw touch values in the serial monitor/plotter
    //    Serial.print(touchRaw[i]);
    //    Serial.print(" ");

    if (touchRaw[i] < touchMin[i]) {
      touchMin[i] = touchRaw[i];
      touchChanged = true;
    }

    if (touchRaw[i] > touchMax[i]) {
      touchMax[i] = touchRaw[i];
      touchChanged = true;
    }

    touch[i] = map(touchRaw[i], touchMin[i], touchMax[i], 0, 255);

    // // uncomment to display mapped/scaled touch values in the serial monitor/plotter
    //    Serial.print(touch[i]);
    //    Serial.print(" ");
  }

  // // uncomment to display raw and/or mapped/scaled touch values in the serial monitor/plotter
  //  Serial.println();

  // uncomment to display raw, scaled, min, max touch values in the serial monitor/plotter
  //  if (touchChanged) {
  //    for (uint8_t i = 0; i < 4; i++) {
  //      Serial.print(touchRaw[i]);
  //      Serial.print(" ");
  //      Serial.print(touch[i]);
  //      Serial.print(" ");
  //      Serial.print(touchMin[i]);
  //      Serial.print(" ");
  //      Serial.print(touchMax[i]);
  //      Serial.print(" ");
  //    }
  //
  //    Serial.println();
  //
  //    touchChanged = false;
  //  }
}

// algorithm from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void drawCircle(int x0, int y0, int radius, const CRGB color, uint8_t thickness = 0)
{
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    addColorXY(x0, y0, color, thickness);
    return;
  }

  while (a >= b)
  {
    addColorXY(a + x0, b + y0, color, thickness);
    addColorXY(b + x0, a + y0, color, thickness);
    addColorXY(-a + x0, b + y0, color, thickness);
    addColorXY(-b + x0, a + y0, color, thickness);
    addColorXY(-a + x0, -b + y0, color, thickness);
    addColorXY(-b + x0, -a + y0, color, thickness);
    addColorXY(a + x0, -b + y0, color, thickness);
    addColorXY(b + x0, -a + y0, color, thickness);

    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

const uint8_t waveCount = 8;

// track the XY coordinates and radius of each wave
uint16_t radii[waveCount];
uint8_t waveX[waveCount];
uint8_t waveY[waveCount];
CRGB waveColor[waveCount];

const uint16_t maxRadius = 512;

void touchDemo() {
  // fade all of the LEDs a small amount each frame
  // increasing this number makes the waves fade faster
  fadeToBlackBy(leds, NUM_LEDS, 30);

  for (uint8_t i = 0; i < touchPointCount; i++) {
    // start new waves when there's a new touch
    if (touch[i] > 127 && radii[i] == 0) {
      radii[i] = 32;
      waveX[i] = touchPointX[i];
      waveY[i] = touchPointY[i];
      waveColor[i] = CHSV(random8(), 255, 255);
    }
  }

  activeWaves = false;

  for (uint8_t i = 0; i < waveCount; i++)
  {
    // increment radii if it's already been set in motion
    if (radii[i] > 0 && radii[i] < maxRadius) radii[i] = radii[i] + 8;

    // reset waves already at max
    if (radii[i] >= maxRadius) {
      activeWaves = true;
      radii[i] = 0;
    }

    if (radii[i] == 0)
      continue;

    activeWaves = true;

    CRGB color = waveColor[i];

    uint8_t x = waveX[i];
    uint8_t y = waveY[i];

    // draw waves starting from the corner closest to each touch sensor
    drawCircle(x, y, radii[i], color, 4);
  }
}

// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void fillWithColorWaves(CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette, bool useFibonacciOrder) {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < fibonacciCount; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16 += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette( palette, index, bri8);

    uint16_t pixelnumber = i;
    if (useFibonacciOrder) pixelnumber = fibonacciToPhysical[i];
    if (pixelnumber >= numleds) continue;
    pixelnumber = (numleds - 1) - pixelnumber;

    nblend(ledarray[pixelnumber], newcolor, 128);
  }
}

void colorWavesFibonacci() {
  fillWithColorWaves(leds, NUM_LEDS, gCurrentPalette, true);
}
