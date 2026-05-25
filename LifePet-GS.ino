#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL  6

#define PIN_DHT      18
#define PIN_LED_HOT  26
#define PIN_LED_OK   27

const float TEMP_MIN = 18.0;
const float TEMP_MAX = 26.0;

const char* WEATHER_URL =
  "https://api.open-meteo.com/v1/forecast"
  "?latitude=-23.5489&longitude=-46.6388"
  "&current=temperature_2m,relativehumidity_2m";

WebServer server(80);
DHTesp dht;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long ultimaLeituraLocal  = 0;
unsigned long ultimaLeituraExterna = 0;
const unsigned long INTERVALO_LOCAL   = 5000;
const unsigned long INTERVALO_EXTERNO = 30000;

float tempLocal  = 0.0;
float umidLocal  = 0.0;
bool  confortavel = false;

float tempExterna = 0.0;
float umidExterna = 0.0;
bool  dadosExternosOk = false;

const int MAX_HISTORICO = 10;
struct Leitura {
  float temp;
  float umid;
  bool  ok;
};
Leitura historico[MAX_HISTORICO];
int totalLeituras = 0;

String boolStr(bool v) {
  return v ? "true" : "false";
}

void atualizarLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pet:");
  lcd.print(tempLocal, 1);
  lcd.print("C ");
  lcd.print(confortavel ? "OK" : "!!");
  lcd.setCursor(0, 1);
  lcd.print("Ext:");
  lcd.print(tempExterna, 1);
  lcd.print("C");
}

void lerSensorLocal() {
  TempAndHumidity dados = dht.getTempAndHumidity();

  if (isnan(dados.temperature) || isnan(dados.humidity)) {
    Serial.println("Falha na leitura do sensor local");
    return;
  }

  tempLocal  = dados.temperature;
  umidLocal  = dados.humidity;
  confortavel = (tempLocal >= TEMP_MIN && tempLocal <= TEMP_MAX);

  digitalWrite(PIN_LED_HOT, confortavel ? LOW : HIGH);
  digitalWrite(PIN_LED_OK,  confortavel ? HIGH : LOW);

  int idx = totalLeituras % MAX_HISTORICO;
  historico[idx] = { tempLocal, umidLocal, confortavel };
  totalLeituras++;

  atualizarLCD();

  Serial.print("Local | Temp: ");
  Serial.print(tempLocal, 1);
  Serial.print("C | Umid: ");
  Serial.print(umidLocal, 0);
  Serial.print("% | Status: ");
  Serial.println(confortavel ? "OK" : "ALERTA");
}

void buscarClimaExterno() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(WEATHER_URL);

  int code = http.GET();

  if (code != 200) {
    Serial.print("Erro API externa: ");
    Serial.println(code);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.println("Erro ao parsear JSON externo");
    return;
  }

  tempExterna = doc["current"]["temperature_2m"];
  umidExterna = doc["current"]["relativehumidity_2m"];
  dadosExternosOk = true;

  atualizarLCD();

  Serial.print("Externo | Temp: ");
  Serial.print(tempExterna, 1);
  Serial.print("C | Umid: ");
  Serial.print(umidExterna, 0);
  Serial.println("%");
}

void handleStatus() {
  String json = "{";
  json += "\"local\":{";
  json += "\"temperatura\":" + String(tempLocal, 1) + ",";
  json += "\"umidade\":"     + String(umidLocal, 1) + ",";
  json += "\"confortavel\":"  + boolStr(confortavel) + ",";
  json += "\"temp_min\":"    + String(TEMP_MIN, 1) + ",";
  json += "\"temp_max\":"    + String(TEMP_MAX, 1);
  json += "},";
  json += "\"externo\":{";
  json += "\"temperatura\":" + String(tempExterna, 1) + ",";
  json += "\"umidade\":"     + String(umidExterna, 1) + ",";
  json += "\"disponivel\":"  + boolStr(dadosExternosOk);
  json += "}";
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleHistorico() {
  String json = "[";
  int total = min(totalLeituras, MAX_HISTORICO);

  for (int i = 0; i < total; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"temp\":"  + String(historico[i].temp, 1) + ",";
    json += "\"umid\":"  + String(historico[i].umid, 1) + ",";
    json += "\"ok\":"    + boolStr(historico[i].ok);
    json += "}";
  }

  json += "]";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "application/json", "{\"erro\":\"rota nao encontrada\"}");
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_HOT, OUTPUT);
  pinMode(PIN_LED_OK,  OUTPUT);
  digitalWrite(PIN_LED_HOT, LOW);
  digitalWrite(PIN_LED_OK,  LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LifePet IoT");
  lcd.setCursor(0, 1);
  lcd.print("Conectando...");

  dht.setup(PIN_DHT, DHTesp::DHT22);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long t = millis();
    while (millis() - t < 100) {}
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  server.on("/api/status",    HTTP_GET, handleStatus);
  server.on("/api/historico", HTTP_GET, handleHistorico);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Servidor HTTP iniciado");

  lerSensorLocal();
  buscarClimaExterno();
}

void loop() {
  server.handleClient();

  unsigned long agora = millis();

  if (agora - ultimaLeituraLocal >= INTERVALO_LOCAL) {
    ultimaLeituraLocal = agora;
    lerSensorLocal();
  }

  if (agora - ultimaLeituraExterna >= INTERVALO_EXTERNO) {
    ultimaLeituraExterna = agora;
    buscarClimaExterno();
  }
}
