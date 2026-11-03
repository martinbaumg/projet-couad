#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "ecran.h"

// Brochage Sprint 1 (alimentation par l'USB du PC)
const int LED_PIN = 16;    // LED rouge 3 mm via résistance 150 Ω (ou 220 Ω)
const int MOTOR_PIN = 4;   // SIG du module vibrant Grove
const int BUZZER_PIN = 17;  // Buzzer (éviter GPIO 0 = boot, et GPIO 6 à 11 = flash interne)

// Réseau Wi-Fi créé par l'ESP32 (point d'accès)
const char* AP_SSID = "COUAD-ESP32";
const char* AP_PASSWORD = "couad2026";  // 8 caractères minimum

WebServer server(80);
bool ledOn = false;
bool motorOn = false;
bool buzzerOn = false;
int gorgees = 0;
TypeBoisson boisson = EAU;

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>COUAD - Contrôle</title>
<style>
  body { font-family: -apple-system, sans-serif; max-width: 420px; margin: 0 auto; padding: 16px; background: #f4f4f6; color: #222; }
  h1 { font-size: 1.4em; text-align: center; }
  .card { background: #fff; border-radius: 12px; padding: 16px; margin-bottom: 14px; box-shadow: 0 1px 4px rgba(0,0,0,.08); }
  .row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }
  .state { font-weight: bold; padding: 4px 10px; border-radius: 20px; background: #ddd; }
  .state.on { background: #2ecc71; color: #fff; }
  .buttons { display: flex; gap: 10px; }
  button { flex: 1; padding: 14px; font-size: 1em; border: none; border-radius: 8px; cursor: pointer; }
  .on-btn { background: #2ecc71; color: #fff; }
  .off-btn { background: #e74c3c; color: #fff; }
  .choice { background: #e4e4e8; color: #222; }
  .choice.sel { background: #3478f6; color: #fff; }
  .drink-btn { background: #3478f6; color: #fff; }
  .label { font-size: .9em; color: #666; margin: 12px 0 6px; }
  button:active { opacity: .7; }
</style>
</head>
<body>
<h1>Contrôle COUAD</h1>

<div class="card">
  <div class="row"><span>LED</span><span id="led" class="state">…</span></div>
  <div class="buttons">
    <button class="on-btn" onclick="setState({led:1})">Allumer</button>
    <button class="off-btn" onclick="setState({led:0})">Éteindre</button>
  </div>
</div>

<div class="card">
  <div class="row"><span>Vibreur</span><span id="motor" class="state">…</span></div>
  <div class="buttons">
    <button class="on-btn" onclick="setState({motor:1})">Allumer</button>
    <button class="off-btn" onclick="setState({motor:0})">Éteindre</button>
  </div>
</div>

<div class="card">
  <div class="row"><span>Buzzer</span><span id="buzzer" class="state">…</span></div>
  <div class="buttons">
    <button class="on-btn" onclick="setState({buzzer:1})">Allumer</button>
    <button class="off-btn" onclick="setState({buzzer:0})">Éteindre</button>
  </div>
</div>

<div class="card">
  <div class="row"><span>Écran</span></div>
  <div class="label">Gorgées à boire</div>
  <div class="buttons">
    <button class="choice" data-gorgees="0" onclick="setState({gorgees:0})">0</button>
    <button class="choice" data-gorgees="1" onclick="setState({gorgees:1})">1</button>
    <button class="choice" data-gorgees="2" onclick="setState({gorgees:2})">2</button>
    <button class="choice" data-gorgees="3" onclick="setState({gorgees:3})">3</button>
  </div>
  <div class="label">Boisson</div>
  <div class="buttons">
    <button class="choice" data-boisson="0" onclick="setState({boisson:0})">Eau</button>
    <button class="choice" data-boisson="1" onclick="setState({boisson:1})">Effort</button>
    <button class="choice" data-boisson="2" onclick="setState({boisson:2})">Énergie</button>
  </div>
  <div class="label"></div>
  <div class="buttons">
    <button class="drink-btn" onclick="setState({bu:1})">J'ai bu une gorgée</button>
  </div>
</div>

<div class="card">
  <div class="row"><span>Tout</span></div>
  <div class="buttons">
    <button class="on-btn" onclick="setState({led:1, motor:1, buzzer:1})">Tout allumer</button>
    <button class="off-btn" onclick="setState({led:0, motor:0, buzzer:0})">Tout éteindre</button>
  </div>
</div>

<script>
function show(s) {
  for (const k of ['led', 'motor', 'buzzer']) {
    const el = document.getElementById(k);
    el.textContent = s[k] ? 'ON' : 'OFF';
    el.classList.toggle('on', !!s[k]);
  }
  for (const k of ['gorgees', 'boisson']) {
    document.querySelectorAll('[data-' + k + ']').forEach(b =>
      b.classList.toggle('sel', b.dataset[k] == s[k]));
  }
}
function setState(params) {
  fetch('/set?' + new URLSearchParams(params)).then(r => r.json()).then(show);
}
fetch('/state').then(r => r.json()).then(show);
</script>
</body>
</html>
)rawliteral";

void applyOutputs() {
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  digitalWrite(MOTOR_PIN, motorOn ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
  Serial.printf("LED %s, vibreur %s, buzzer %s\n",
                ledOn ? "ON" : "OFF", motorOn ? "ON" : "OFF", buzzerOn ? "ON" : "OFF");
}

void sendState() {
  String json = String("{\"led\":") + (ledOn ? 1 : 0) + ",\"motor\":" + (motorOn ? 1 : 0) +
                ",\"buzzer\":" + (buzzerOn ? 1 : 0) +
                ",\"gorgees\":" + gorgees + ",\"boisson\":" + boisson + "}";
  server.send(200, "application/json", json);
}

void handleSet() {
  if (server.hasArg("led")) ledOn = server.arg("led") == "1";
  if (server.hasArg("motor")) motorOn = server.arg("motor") == "1";
  if (server.hasArg("buzzer")) buzzerOn = server.arg("buzzer") == "1";
  applyOutputs();

  int anciennesGorgees = gorgees;
  TypeBoisson ancienneBoisson = boisson;
  if (server.hasArg("gorgees")) gorgees = constrain(server.arg("gorgees").toInt(), 0, GORGEES_MAX);
  if (server.hasArg("boisson")) boisson = (TypeBoisson)constrain(server.arg("boisson").toInt(), 0, (int)ENERGIE);
  if (server.hasArg("bu") && gorgees > 0) gorgees--;
  // Redessine l'écran seulement si quelque chose a changé
  if (gorgees != anciennesGorgees || boisson != ancienneBoisson) {
    ecranAfficher(gorgees, boisson);
    Serial.printf("Écran : %d gorgée(s), boisson %d\n", gorgees, boisson);
  }
  sendState();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  applyOutputs();
  ecranInit();
  ecranAfficher(gorgees, boisson);

  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("Wi-Fi \"%s\" créé, page sur http://%s\n", AP_SSID, WiFi.softAPIP().toString().c_str());

  server.on("/", []() { server.send_P(200, "text/html; charset=utf-8", PAGE); });
  server.on("/state", sendState);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  server.handleClient();
}
