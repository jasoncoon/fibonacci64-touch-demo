
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

  for ( uint16_t i = 0 ; i < numleds; i++) {
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

    pixelnumber = (numleds - 1) - pixelnumber;

    nblend(ledarray[pixelnumber], newcolor, 128);
  }
}

void colorWavesFibonacci() {
  fillWithColorWaves(leds, NUM_LEDS, gCurrentPalette, true);
}

// Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void fillWithPride(bool useFibonacciOrder)
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t sat8 = beatsin88( 43.5, 220, 250);
  // uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint8_t brightdepth = beatsin88(171, 96, 224);
  // uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint16_t brightnessthetainc16 = beatsin88( 102, (25 * 256), (40 * 256));
  // uint8_t msmultiplier = beatsin88(147, 23, 60);
  uint8_t msmultiplier = beatsin88(74, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  // uint16_t hueinc16 = beatsin88(113, 1, 3000);
  uint16_t hueinc16 = beatsin88(57, 1, 128);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  // sHue16 += deltams * beatsin88( 400, 5, 9);
  sHue16 += deltams * beatsin88( 200, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;

    if (useFibonacciOrder) pixelnumber = fibonacciToPhysical[i];

    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}

void prideFibonacci() {
  fillWithPride(true);
}

void outwardPalettes() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = ColorFromPalette(gCurrentPalette, physicalToFibonacci[i] - hue);
  }
}

void rotatingPalettes() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = ColorFromPalette(gCurrentPalette, angles[i] - hue);
  }
}

void outwardRainbow() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = CHSV(physicalToFibonacci[i] - hue, 255, 255);
  }
}

void rotatingRainbow() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = CHSV(angles[i] - hue, 255, 255);
  }
}

void horizontalRainbow() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = CHSV(coordsX[i] + hue, 255, 255);
  }
}

void verticalRainbow() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = CHSV(coordsY[i] + hue, 255, 255);
  }
}

void diagonalRainbow() {
  for (uint16_t i = 0; i < NUM_LEDS; i++) { 
    leds[i] = CHSV(coordsX[i] + coordsY[i] + hue, 255, 255);
  }
}

void colorTest() {
  CRGB colors[] = { CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::White, CRGB::Black };
  const uint8_t colorCount = ARRAY_SIZE(colors);
  static uint8_t colorIndex = 0;
  EVERY_N_SECONDS(2) { colorIndex = (colorIndex + 1) % colorCount; }
  fill_solid(leds, NUM_LEDS, colors[colorIndex]);
}
