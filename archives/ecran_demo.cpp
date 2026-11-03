// Démo de l'écran (utilise src/ecran.cpp) : à remettre dans src/main.cpp pour la lancer
#include <Arduino.h>
#include "ecran.h"

// On fait défiler les alertes comme dans la maquette Wokwi
struct Alerte {
  int nbGorgees;
  TypeBoisson boisson;
};

const Alerte ALERTES[] = {
  {1, EAU},      // 1 gorgée d'eau : goutte bleue
  {2, EFFORT},   // 2 gorgées de boisson d'effort : goutte rouge
  {3, ENERGIE},  // 3 gorgées de boisson énergie : goutte jaune
};
const int NB_ALERTES = sizeof(ALERTES) / sizeof(ALERTES[0]);

int etape = 0;

void setup() {
  Serial.begin(115200);
  ecranInit();
}

void loop() {
  ecranAfficher(ALERTES[etape].nbGorgees, ALERTES[etape].boisson);
  Serial.printf("Alerte : %d gorgée(s), boisson %d\n", ALERTES[etape].nbGorgees, ALERTES[etape].boisson);

  etape = (etape + 1) % NB_ALERTES;
  delay(2500);  // Chaque alerte reste affichée 2,5 secondes
}
