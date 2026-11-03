# projet-couad

Prototype d'un rappel d'hydratation sur ESP32 : le boîtier signale qu'il faut boire (LED, vibreur, buzzer) et affiche sur un petit écran le nombre de gorgées à prendre et le type de boisson (eau, effort, énergie).

## Matériel

- ESP32 WROOM (carte `esp32dev`)
- LED rouge 3 mm + résistance
- Module moteur vibrant Grove
- Buzzer
- Écran Joy-IT 1,8" ST7735 (160×128, SPI)

Le schéma électrique et le calcul de la résistance sont dans [docs/sprint1_schema_electrique.pdf](docs/sprint1_schema_electrique.pdf).

## Organisation du code

```
src/
  main.cpp              Sprint 1 : contrôle de la LED, du vibreur et du buzzer depuis une page web
  ecran.h / ecran.cpp   Affichage des gorgées et du type de boisson sur l'écran ST7735
  diag/diag_ecran.cpp   Programme de diagnostic de l'écran (câblage, mode SPI)
archives/
  ecran_demo.cpp        Démo qui fait défiler des alertes sur l'écran
  sprint1_controle_web.cpp  Version de la page web qui pilote aussi l'écran
```

## Compiler et téléverser

Avec [PlatformIO](https://platformio.org/) :

```sh
pio run -t upload -t monitor            # programme principal
pio run -e diag -t upload -t monitor    # diagnostic de l'écran
```

## Utilisation (Sprint 1)

1. Alimenter l'ESP32 : il crée le réseau Wi-Fi `COUAD-ESP32` (mot de passe `couad2026`).
2. Se connecter à ce réseau depuis un téléphone ou un PC.
3. Ouvrir http://192.168.4.1 pour allumer ou éteindre la LED, le vibreur et le buzzer.

## Brochage

| Composant | Broche ESP32 |
|---|---|
| LED | GPIO 16 |
| Vibreur (SIG) | GPIO 4 |
| Buzzer | GPIO 17 |
| Écran CS | GPIO 5 |
| Écran DC | GPIO 16 |
| Écran RST | GPIO 17 |
| Écran SCK | GPIO 18 |
| Écran SDA/MOSI | GPIO 23 |
