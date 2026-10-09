#include <FastLED.h>

#define LED_PIN      6
#define MIC_PIN      A1
#define NUM_LEDS     300       // À remplacer par le nombre réel de LED
#define LED_TYPE     WS2812B  // Protocole compatible utilisé par FastLED
#define COLOR_ORDER  GRB

CRGB leds[NUM_LEDS];

const unsigned long WINDOW_MS = 20; // Fenêtre de mesure
const int NOISE_FLOOR = 8;          // À ajuster selon le microphone
const int MAX_LEVEL = 180;          // Niveau sonore correspondant à la pleine luminosité

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(180);
  FastLED.clear(true);
  Serial.begin(115200);
}

void loop() {
  int signalMin = 1023;
  int signalMax = 0;
  unsigned long start = millis();

  // Mesure l'amplitude crête-à-crête pendant environ 20 ms
  while (millis() - start < WINDOW_MS) {
    int sample = analogRead(MIC_PIN);
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
