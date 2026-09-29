#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>
#include "secrets.h"

EnergyMonitor sensor;

const byte SENSOR_PIN = A0;
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
  {IOT002_WRITE_KEY, "iot002-pessoal"},
  {IOT002_IC_WRITE_KEY, "iot002-IC"}
};
DestinationStats stats[2] = {};
const byte DESTINATION_COUNT = sizeof(destinations) / sizeof(destinations[0]);

byte mac[] = {0x02, 0x60, 0x37, 0x12, 0x34, 0x80};
EthernetClient client;
bool ethernetReady = false;
unsigned long lastDhcpAttempt = 0;
unsigned long lastSend = 0;

void connectEthernet() {
  Serial.println("Solicitando endereco IP por DHCP...");
  lastDhcpAttempt = millis();
  if (Ethernet.begin(mac, 5000, 1000) == 0) {
    ethernetReady = false;
    Serial.println("Falha no DHCP. Verifique cabo e rede.");
    return;
  }

  ethernetReady = true;
  delay(1000);
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
}

void sendToThingSpeak(byte index, double currentRms) {
  Destination destination = destinations[index];
  DestinationStats& destinationStats = stats[index];

  if (!ethernetReady || Ethernet.linkStatus() == LinkOFF) {
    destinationStats.failureCount++;
    Serial.print(destination.name);
    Serial.println(": sem Ethernet disponivel.");
    return;
  }

  if (!client.connect(THINGSPEAK_HOST, 80)) {
    destinationStats.failureCount++;
    Serial.print(destination.name);
    Serial.println(": falha ao conectar ao ThingSpeak.");
    return;
  }

  String request = "GET /update?api_key=";
  request += destination.writeKey;
  request += "&field1=";
  request += String(currentRms, 3);
  request += "&field2=";
  request += String(millis() / 1000UL);
  request += "&field3=";
  request += String(destinationStats.lastLatency);
  request += "&field4=";
  request += String(destinationStats.totalBytesSent);
  request += "&field5=";
  request += String(destinationStats.lastRate, 1);
  request += "&field6=";
  request += String(destinationStats.successCount);
  request += "&field7=";
  request += String(destinationStats.failureCount);
  request += "&field8=1&status=iot002%20IP%3A";
  request += Ethernet.localIP()[0];
  request += ".";
  request += Ethernet.localIP()[1];
  request += ".";
  request += Ethernet.localIP()[2];
  request += ".";
  request += Ethernet.localIP()[3];
  request += " HTTP/1.1\r\nHost: ";
  request += THINGSPEAK_HOST;
  request += "\r\nConnection: close\r\n\r\n";

  unsigned long started = millis();
  client.print(request);
  destinationStats.totalBytesSent += request.length();

  while (client.connected() && millis() - started < 5000) {
    while (client.available()) {
      client.read();
    }
  }

  destinationStats.lastLatency = millis() - started;
  destinationStats.lastRate = request.length() * 1000.0 / max(1UL, destinationStats.lastLatency);
  destinationStats.successCount++;
  client.stop();

  Serial.print(destination.name);
  Serial.print(": dados enviados, latencia ");
  Serial.print(destinationStats.lastLatency);
  Serial.println(" ms");
}

void setup() {
  Serial.begin(9600);
  sensor.current(SENSOR_PIN, 6.0606);
  Ethernet.init(10);
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

  if (!ethernetReady && (lastDhcpAttempt == 0 || millis() - lastDhcpAttempt >= 30000UL)) {
    connectEthernet();
  }

  if (millis() - lastSend >= SEND_INTERVAL) {
    for (byte index = 0; index < DESTINATION_COUNT; index++) {
      sendToThingSpeak(index, currentRms);
    }
    lastSend = millis();
  }

  Serial.println();
  delay(2000);
}
