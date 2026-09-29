#include "EmonLib.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "../../secrets.h"

EnergyMonitor sensor;

const int SENSOR_PIN = 36; // GPIO36, ADC1/ADC0
const float MAINS_VOLTAGE = 127.0;
const char THINGSPEAK_HOST[] = "api.thingspeak.com";
const unsigned long SEND_INTERVAL = 30000;

struct Destination {
  const char* writeKey;
  const char* name;
};

struct DestinationStats {
  unsigned long successCount;
  unsigned long failureCount;
  unsigned long totalBytesSent;
  unsigned long lastLatency;
  float lastRate;
};

Destination destinations[] = {
  {IOT003_WRITE_KEY, "iot003-pessoal"},
  {IOT003_IC_WRITE_KEY, "iot003-IC"}
};
DestinationStats stats[2] = {};
const byte DESTINATION_COUNT = sizeof(destinations) / sizeof(destinations[0]);

unsigned long lastSend = 0;

void printNetworkInfo() {
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC = ");
  Serial.println(WiFi.macAddress());
}

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
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println(" falhou");
  }
}

void sendToThingSpeak(byte index, double currentRms) {
  Destination destination = destinations[index];
  DestinationStats& destinationStats = stats[index];

  if (WiFi.status() != WL_CONNECTED) {
    destinationStats.failureCount++;
    Serial.print(destination.name);
    Serial.println(": sem Wi-Fi; dados nao enviados.");
    return;
  }

  HTTPClient http;
  String url = "http://";
  url += THINGSPEAK_HOST;
  url += "/update?api_key=";
  url += destination.writeKey;
  url += "&field1=";
  url += String(currentRms, 3);
  url += "&field2=";
  url += String(millis() / 1000UL);
  url += "&field3=";
  url += String(destinationStats.lastLatency);
  url += "&field4=";
  url += String(destinationStats.totalBytesSent);
  url += "&field5=";
  url += String(destinationStats.lastRate, 1);
  url += "&field6=";
  url += String(destinationStats.successCount);
  url += "&field7=";
  url += String(destinationStats.failureCount);
  url += "&field8=1&status=iot003%20IP%3A";
  url += WiFi.localIP().toString();

  unsigned long started = millis();
  http.begin(url);
  int responseCode = http.GET();
  String response = http.getString();
  http.end();

  destinationStats.lastLatency = millis() - started;
  destinationStats.totalBytesSent += url.length();
  destinationStats.lastRate = url.length() * 1000.0 / max(1UL, destinationStats.lastLatency);

  if (responseCode == HTTP_CODE_OK && response.toInt() > 0) {
    destinationStats.successCount++;
    Serial.print(destination.name);
    Serial.print(": dados enviados. Entry: ");
    Serial.println(response);
  } else {
    destinationStats.failureCount++;
    Serial.print(destination.name);
    Serial.print(": falha HTTP ");
    Serial.println(responseCode);
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

  printNetworkInfo();
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
    for (byte index = 0; index < DESTINATION_COUNT; index++) {
      sendToThingSpeak(index, currentRms);
    }
    lastSend = millis();
  }

  delay(2000);
}
