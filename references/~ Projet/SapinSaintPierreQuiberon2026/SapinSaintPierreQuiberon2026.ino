//Programme pour le jeu de vélo qui commande une guirlande électrique installée sur le sapin de Noël 2026 
//de la commune de Saint-Pierre-Quiberon.
//Programme à installer sur une carte ARDUINO UNO, à laquelle sont réliées deux guirlandes de 5m et 300 DEL chacune.
//Le vélo, sur un support fixe, entraîne un générateur électrique. Celui-ci fournit une tension proportionnelle
//à la vitesse. Plus la vitesse augmente, plus l'effet lumineux de la guirlande est important (en amplitude et en brillance).
//Un grand afficheur de 2 caractères est connecté à l'Arduino pour indiquer la quantité de calories dépensées.
//Au repos, la guirlande affiche un défilement régulier des LED et le son est silencieux. Pour démarrer, le cycliste s'installe sur le vélo et appui sur un bouton 
//"démarrer". Le cycliste pédale pendant une durée de 3 mn. La guirlande s'anime et l'afficheur indique la progression des calories. L'ampli diffuse une musique 
//d'encouragement (style "Allez les bleus" ou autre). 
//Au bout des 3 mn, l'afficheur fige la valeur en calories dépensées et la guirlande affiche un effet lumineux de feu d'artifice pendant 30 secondes. 
//En même temps, l'ampli diffuse des applaudissements, puis une musique de vaincoeur.
//Puis, on revient au programme de départ.

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"                      //Librairie pour le lecteur MP3

#include <FastLED.h>                                  //Librairies pour l'animation des LEDs
#include <RobotixLEDEffects.h>

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN      6          //Port de sortie de la carte ARDUINO pour commander les LED
#define GEN_PIN      A1         //Port d'entrée de la carte ARDUINO pour mesurer le signal du générateur
#define NUM_LEDS     300       // À remplacer par le nombre réel de LED
#define LED_TYPE     WS2812B  // Protocole compatible utilisé par FastLED
#define COLOR_ORDER  GRB

CRGB leds[NUM_LEDS];
RobotixLEDEffects effets(leds, NUM_LEDS);

const unsigned long WINDOW_MS = 20; // Fenêtre de mesure
const int NOISE_FLOOR = 8;          // À ajuster selon le microphone
const int MAX_LEVEL = 180;          // Niveau sonore correspondant à la pleine luminosité

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(180);
  FastLED.clear(true);
  FastLED.show();
  Serial.begin(115200);
}

void loop() {
   effets.DefilementArcEnCiel(50);
  
  int signalMin = 1023;
  int signalMax = 0;
  unsigned long start = millis();

  // Mesure l'amplitude crête-à-crête pendant environ 20 ms
  while (millis() - start < WINDOW_MS) {
    int sample = analogRead(GEN_PIN);
    if (sample < signalMin) signalMin = sample;
    if (sample > signalMax) signalMax = sample;
  }

  int amplitude = signalMax - signalMin;
  int level = max(0, amplitude - NOISE_FLOOR);
  int brightness = constrain(map(level, 0, MAX_LEVEL, 0, 255), 0, 255);

  // Couleur qui évolue avec le niveau sonore
  uint8_t hue = map(constrain(level, 0, MAX_LEVEL), 0, MAX_LEVEL, 160, 0);
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, brightness));
  FastLED.show();

  Serial.print("Amplitude: ");
  Serial.print(amplitude);
  Serial.print("  Luminosite: ");
  Serial.println(brightness);
}

