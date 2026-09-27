#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>
#include "secrets.h"

EnergyMonitor sensor;

// ThingSpeak channel 3510600: Field 1 is "Corrente (A)".
const byte SENSOR_PIN = A0;
const float MAINS_VOLTAGE = 127.0;
const unsigned long THINGSPEAK_CHANNEL_ID = 3510600;
const char THINGSPEAK_HOST[] = "api.thingspeak.com";
const char THINGSPEAK_WRITE_KEY[] = IOT002_WRITE_KEY;
const unsigned long SEND_INTERVAL = 30000;

byte mac[] = {0x02, 0x60, 0x37, 0x12, 0x34, 0x80};
EthernetClient client;
unsigned long lastSend = 0;
unsigned long successCount = 0;
unsigned long failureCount = 0;
unsigned long totalBytesSent = 0;
unsigned long lastLatency = 0;
float lastRate = 0;
bool ethernetReady = false;
unsigned long lastDhcpAttempt = 0;

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

void sendToThingSpeak(double currentRms) {
  if (!ethernetReady) {
    failureCount++;
    Serial.println("Sem IP Ethernet; dados nao enviados.");
    return;
  }

  bool linkUp = Ethernet.linkStatus() != LinkOFF;
  if (!linkUp) {
    ethernetReady = false;
    failureCount++;
    Serial.println("Cabo Ethernet desconectado.");
    return;
  }

  if (!client.connect(THINGSPEAK_HOST, 80)) {
    failureCount++;
    Serial.println("Falha ao conectar ao ThingSpeak.");
    return;
  }

  String request = "GET /update?api_key=";
  request += THINGSPEAK_WRITE_KEY;
  request += "&field1=";
  request += String(currentRms, 3);
  request += "&field2=";
  request += String(millis() / 1000UL);
  request += "&field3=";
  request += String(lastLatency);
  request += "&field4=";
  request += String(totalBytesSent);
  request += "&field5=";
  request += String(lastRate, 1);
  request += "&field6=";
  request += String(successCount);
  request += "&field7=";
  request += String(failureCount);
  request += "&field8=";
  request += linkUp ? "1" : "0";
  request += "&status=IP%3A";
  IPAddress ip = Ethernet.localIP();
  request += String(ip[0]);
  request += ".";
  request += String(ip[1]);
  request += ".";
  request += String(ip[2]);
  request += ".";
  request += String(ip[3]);
  request += " HTTP/1.1\r\nHost: ";
  request += THINGSPEAK_HOST;
  request += "\r\nConnection: close\r\n\r\n";

  unsigned long started = millis();
  client.print(request);
  totalBytesSent += request.length();

  while (client.connected() && millis() - started < 5000) {
    while (client.available()) {
      client.read();
    }
  }
  lastLatency = millis() - started;
  lastRate = request.length() * 1000.0 / max(1UL, lastLatency);
  successCount++;
  client.stop();
  Serial.print("Dados enviados. Latencia: ");
  Serial.print(lastLatency);
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
    sendToThingSpeak(currentRms);
    lastSend = millis();
  }

  Serial.println();
  delay(2000);
}
