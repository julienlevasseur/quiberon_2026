#include <Adafruit_NeoPixel.h>
#include <RobotixLEDEffects.h>

#define NUM_LEDS 300
//#define PIN D6
#define PIN 6        //modif COR

Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
RobotixLEDEffects effets(strip);

void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  effets.RGBLoop();

  effets.FadeInOut(0xff, 0x00, 0x00); // rouge
  effets.FadeInOut(0xff, 0xff, 0xff); // blanc
  effets.FadeInOut(0x00, 0x00, 0xff); // bleu

  effets.Strobe(0xFF, 0xFF, 0xFF, 10, 50, 1000);

  effets.HalloweenEyes(0xff, 0x00, 0x00, 1, 4, true, 20, 100, 1000);

  effets.CylonBounce(0xff, 0x00, 0x00, 4, 10, 50);

  effets.NewKITT(0xff, 0x00, 0x00, 4, 10, 50);

  effets.Twinkle(0xff, 0x00, 0x00, 10, 100, false);

  effets.TwinkleRandom(20, 100, false);

  effets.Sparkle(0xff, 0xff, 0xff, 0);

  effets.SnowSparkle(0x10, 0x10, 0x10, 20, 200);

  effets.RunningLights(0xff, 0x00, 0x00, 50);

  effets.colorWipe(0x00, 0xff, 0x00, 50);
  effets.colorWipe(0x00, 0x00, 0xff, 50);
  effets.colorWipe(0xff, 0x00, 0x00, 50);

  effets.rainbowCycle(5);

  effets.theaterChase(0xff, 0x00, 0x00, 50);
  effets.theaterChaseRainbow(50);

  effets.Fire(55, 120, 15);

  byte uneBalle[1][3] = { {0xff, 0x00, 0x00} };
  effets.BouncingColoredBalls(1, uneBalle, false);

  byte couleurs[3][3] = { {0xff, 0x00, 0x00}, {0xff, 0xff, 0xff}, {0x00, 0x00, 0xff} };
  effets.BouncingColoredBalls(3, couleurs, false);

  effets.meteorRain(0xff, 0xff, 0xff, 10, 64, true, 0);
}
