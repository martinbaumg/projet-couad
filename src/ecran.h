#pragma once
#include <Arduino.h>

// Type de boisson à prendre, affiché par la couleur de la goutte
enum TypeBoisson { EAU, EFFORT, ENERGIE };

const int GORGEES_MAX = 3;

void ecranInit();
// Affiche le nombre de gorgées à boire (0 à GORGEES_MAX) et le type de boisson
void ecranAfficher(int nbGorgees, TypeBoisson boisson);
