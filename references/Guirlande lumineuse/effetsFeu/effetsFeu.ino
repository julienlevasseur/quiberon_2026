#include <Adafruit_NeoPixel.h>
#include <RobotixLEDEffects.h>


#define NB_LEDS 300
#define BROCHE 6

Adafruit_NeoPixel strip(NB_LEDS, BROCHE, NEO_GRB + NEO_KHZ800);
RobotixLEDEffects effets(strip);

void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  effets.Feu(55, 120, 15);
}