#include "ecran.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Écran Joy-IT 1.8" ST7735 (160x128) sur le bus SPI matériel : SCK = IO18, SDA/MOSI = IO23
#define TFT_CS   5   // Fil JAUNE  -> IO5
#define TFT_DC  16   // Fil ORANGE -> IO16 (RS/DC)
#define TFT_RST 17   // Fil BLANC  -> IO17 (RES)

static Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

static const int LARGEUR = 160;
static const int HAUTEUR = 128;
static const uint16_t GRIS = 0x4208;

// Couleur et nom affichés pour chaque type de boisson
static const uint16_t COULEURS[] = {ST77XX_BLUE, ST77XX_RED, ST77XX_YELLOW};
static const char* NOMS[] = {"EAU", "EFFORT", "ENERGIE"};

// Goutte : pointe en (x, y), partie ronde de rayon 12 en dessous
static void dessinerGoutte(int x, int y, uint16_t couleur) {
  tft.fillTriangle(x, y, x - 12, y + 24, x + 12, y + 24, couleur);
  tft.fillCircle(x, y + 24, 12, couleur);
  // Reflet blanc pour le relief
  tft.fillCircle(x - 4, y + 20, 3, ST77XX_WHITE);
}

// Écrit un texte centré horizontalement sur centreX
static void texteCentre(const char* texte, int centreX, int y, uint8_t taille, uint16_t couleur) {
  int16_t x1, y1;
  uint16_t l, h;
  tft.setTextSize(taille);
  tft.getTextBounds(texte, 0, 0, &x1, &y1, &l, &h);
  tft.setCursor(centreX - l / 2, y);
  tft.setTextColor(couleur);
  tft.print(texte);
}

void ecranInit() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);  // Paysage : 160 de large, 128 de haut
  tft.fillScreen(ST77XX_BLACK);
}

void ecranAfficher(int nbGorgees, TypeBoisson boisson) {
  nbGorgees = constrain(nbGorgees, 0, GORGEES_MAX);
  tft.fillScreen(ST77XX_BLACK);

  // Barres à gauche : toujours GORGEES_MAX emplacements pour que rien ne bouge,
  // pleines pour les gorgées restant à boire, en contour gris pour les autres
  const int barreX = 14, barreLargeur = 65, barreHauteur = 14, espacement = 10;
  const int hauteurTotale = GORGEES_MAX * barreHauteur + (GORGEES_MAX - 1) * espacement;
  const int startY = (105 - hauteurTotale) / 2;
  for (int i = 0; i < GORGEES_MAX; i++) {
    int y = startY + i * (barreHauteur + espacement);
    if (i < nbGorgees) {
      tft.fillRoundRect(barreX, y, barreLargeur, barreHauteur, 3, ST77XX_WHITE);
    } else {
      tft.drawRoundRect(barreX, y, barreLargeur, barreHauteur, 3, GRIS);
    }
  }

  // Goutte à droite, grisée quand il n'y a rien à boire
  uint16_t couleur = nbGorgees > 0 ? COULEURS[boisson] : GRIS;
  dessinerGoutte(122, 22, couleur);
  texteCentre(NOMS[boisson], 122, 66, 1, couleur);

  // Message en bas
  char message[16];
  if (nbGorgees == 0) {
    strcpy(message, "BIEN JOUE !");
  } else {
    snprintf(message, sizeof(message), "%d GORGEE%s", nbGorgees, nbGorgees > 1 ? "S" : "");
  }
  texteCentre(message, LARGEUR / 2, HAUTEUR - 22, 2, nbGorgees > 0 ? ST77XX_WHITE : ST77XX_GREEN);
}
