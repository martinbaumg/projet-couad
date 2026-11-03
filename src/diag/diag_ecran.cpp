// Programme de diagnostic de l'écran ST7735 (environnement PlatformIO "diag")
//   pio run -e diag -t upload -t monitor
// 1. Vérifie qu'aucune broche de l'écran n'est en court-circuit
// 2. Essaie plusieurs façons de piloter l'écran : chaque mode affiche "MODE n"
//    sur fond rouge, vert puis bleu. Le mode qui s'affiche indique la bonne config.
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS    5
#define TFT_DC   16
#define TFT_RST  17
#define TFT_MOSI 23
#define TFT_SCLK 18

const int PINS[] = {TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK};
const char* NOMS_PINS[] = {"CS (IO5)", "A0/DC (IO16)", "RESET (IO17)", "SDA (IO23)", "SCK (IO18)"};
const int NB_PINS = sizeof(PINS) / sizeof(PINS[0]);

struct Mode {
  const char* nom;
  bool spiLogiciel;
  uint8_t onglet;
};

const Mode MODES[] = {
  {"SPI materiel 32 MHz, BLACKTAB (config actuelle)", false, INITR_BLACKTAB},
  {"SPI logiciel (lent), BLACKTAB", true, INITR_BLACKTAB},
  {"SPI logiciel (lent), GREENTAB", true, INITR_GREENTAB},
  {"SPI logiciel (lent), REDTAB", true, INITR_REDTAB},
};
const int NB_MODES = sizeof(MODES) / sizeof(MODES[0]);

void testerBroches() {
  Serial.println("\n=== Test des broches de l'ecran ===");
  bool probleme = false;

  // Court-circuit entre deux broches : l'une à 3,3 V fait monter l'autre
  for (int i = 0; i < NB_PINS; i++) {
    for (int j = 0; j < NB_PINS; j++) pinMode(PINS[j], INPUT_PULLDOWN);
    pinMode(PINS[i], OUTPUT);
    digitalWrite(PINS[i], HIGH);
    delay(5);
    for (int j = i + 1; j < NB_PINS; j++) {
      if (digitalRead(PINS[j]) == HIGH) {
        Serial.printf("  PROBLEME : court-circuit entre %s et %s\n", NOMS_PINS[i], NOMS_PINS[j]);
        probleme = true;
      }
    }
  }

  // Broche reliée à la masse ou tirée vers le 3,3 V
  for (int i = 0; i < NB_PINS; i++) {
    pinMode(PINS[i], INPUT_PULLUP);
    delay(5);
    if (digitalRead(PINS[i]) == LOW) {
      Serial.printf("  PROBLEME : %s est reliee a GND\n", NOMS_PINS[i]);
      probleme = true;
    }
    pinMode(PINS[i], INPUT_PULLDOWN);
    delay(5);
    if (digitalRead(PINS[i]) == HIGH) {
      Serial.printf("  Info : %s est tiree vers 3,3 V (normal si l'ecran a une resistance de tirage dessus)\n",
                    NOMS_PINS[i]);
    }
    pinMode(PINS[i], INPUT);
  }

  if (!probleme) Serial.println("  Aucun court-circuit ni broche a la masse detecte");
}

void essayerMode(int n) {
  const Mode& mode = MODES[n];
  Serial.printf("\n>>> MODE %d : %s -> regarde l'ecran\n", n, mode.nom);

  Adafruit_ST7735* tft = mode.spiLogiciel
      ? new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST)
      : new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
  tft->initR(mode.onglet);
  tft->setRotation(1);

  const uint16_t FONDS[] = {ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE};
  for (uint16_t fond : FONDS) {
    tft->fillScreen(fond);
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(4);
    tft->setCursor(10, 45);
    tft->printf("MODE %d", n);
    delay(1500);
  }

  delete tft;
  // Libère IO18/IO23 pour que le mode logiciel suivant puisse les piloter
  if (!mode.spiLogiciel) SPI.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  testerBroches();
}

void loop() {
  for (int n = 0; n < NB_MODES; n++) essayerMode(n);
  Serial.println("\n--- Fin du cycle, on recommence ---");
}
