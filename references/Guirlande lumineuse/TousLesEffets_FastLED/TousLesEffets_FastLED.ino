// =============================================
// Exemple avec FastLED (ex: ESP32-C3)
// Inclure FastLED AVANT RobotixLEDEffects
// =============================================

#include <FastLED.h>
#include <RobotixLEDEffects.h>

#define NB_LEDS 300
#define BROCHE 6    // numéro de broche GPIO

CRGB leds[NB_LEDS];
RobotixLEDEffects effets(leds, NB_LEDS);

void setup() {
  FastLED.addLeds<WS2812B, BROCHE, GRB>(leds, NB_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  //effets.BoucleRGB();
  //effets.Fondu(0xff, 0x00, 0x00);           // rouge
  //effets.Fondu(0xff, 0xff, 0xff);           // blanc
  //effets.Fondu(0x00, 0x00, 0xff);           // bleu
  //effets.Stroboscope(0xFF, 0xFF, 0xFF, 10, 50, 1000);
  //effets.YeuxHalloween(0xff, 0x00, 0x00, 1, 4, true, 20, 100, 1000);
  //effets.BalayageCylon(0xff, 0x00, 0x00, 4, 10, 50);
  //effets.BalayageKITT(0xff, 0x00, 0x00, 4, 10, 50);
  //effets.Scintillement(0xff, 0x00, 0x00, 10, 100, false);
  //effets.ScintillementAleatoire(20, 100, false);
  //effets.Etincelle(0xff, 0xff, 0xff, 0);
  //effets.EtincelleNeige(0x10, 0x10, 0x10, 20, 200);
  //effets.LumiereCourante(0xff, 0x00, 0x00, 50);
  //effets.RemplissageCouleur(0x00, 0xff, 0x00, 50);
  //effets.ArcEnCiel(5);
  //effets.DefilementTheatre(0xff, 0x00, 0x00, 50);
  effets.DefilementArcEnCiel(50);
  //effets.Feu(55, 120, 15);

  byte couleurs[3][3] = { {0xff, 0x00, 0x00}, {0xff, 0xff, 0xff}, {0x00, 0x00, 0xff} };
  //effets.BallesRebondissantes(3, couleurs, false);

  //effets.PluieMeteore(0xff, 0xff, 0xff, 10, 64, true, 0);
}
