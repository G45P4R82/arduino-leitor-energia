#include "EmonLib.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "../../secrets.h"

EnergyMonitor sensor;

const int SENSOR_PIN = 36; // GPIO36, ADC1/ADC0
const float MAINS_VOLTAGE = 127.0;
const char THINGSPEAK_HOST[] = "api.thingspeak.com";
const char THINGSPEAK_WRITE_KEY[] = IOT003_WRITE_KEY;
const unsigned long SEND_INTERVAL = 30000;

unsigned long lastSend = 0;
unsigned long successCount = 0;
unsigned long failureCount = 0;
unsigned long totalBytesSent = 0;
unsigned long lastLatency = 0;
float lastRate = 0;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Conectando ao Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(" conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" falhou");
  }
}

void sendToThingSpeak(double currentRms) {
  if (WiFi.status() != WL_CONNECTED) {
    failureCount++;
    Serial.println("Sem Wi-Fi; dados nao enviados.");
    return;
  }

  HTTPClient http;
  String url = "http://";
  url += THINGSPEAK_HOST;
  url += "/update?api_key=";
  url += THINGSPEAK_WRITE_KEY;
  url += "&field1=";
  url += String(currentRms, 3);
  url += "&field2=";
  url += String(millis() / 1000UL);
  url += "&field3=";
  url += String(lastLatency);
  url += "&field4=";
  url += String(totalBytesSent);
  url += "&field5=";
  url += String(lastRate, 1);
  url += "&field6=";
  url += String(successCount);
  url += "&field7=";
  url += String(failureCount);
  url += "&field8=1&status=IP%3A";
  url += WiFi.localIP().toString();

  unsigned long started = millis();
  http.begin(url);
  int responseCode = http.GET();
  String response = http.getString();
  http.end();

  lastLatency = millis() - started;
  totalBytesSent += url.length();
  lastRate = url.length() * 1000.0 / max(1UL, lastLatency);

  if (responseCode == HTTP_CODE_OK && response.toInt() > 0) {
    successCount++;
    Serial.print("Dados enviados. Entry: ");
    Serial.println(response);
  } else {
    failureCount++;
    Serial.print("Falha ThingSpeak HTTP ");
    Serial.print(responseCode);
    Serial.print(" resposta: ");
    Serial.println(response);
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);
  analogSetPinAttenuation(SENSOR_PIN, ADC_11db);
  sensor.current(SENSOR_PIN, 6.0606);
  connectWiFi();
}

void loop() {
  double currentRms = sensor.calcIrms(1480);
  double apparentPower = currentRms * MAINS_VOLTAGE;

  Serial.print("Corrente = ");
  Serial.print(currentRms, 3);
  Serial.println(" A");
  Serial.print("Potencia aparente = ");
  Serial.print(apparentPower, 1);
  Serial.println(" VA");

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (millis() - lastSend >= SEND_INTERVAL) {
    sendToThingSpeak(currentRms);
    lastSend = millis();
  }

  delay(2000);
}
