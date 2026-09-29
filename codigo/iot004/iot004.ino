#include <SPI.h>
#include <Ethernet.h>
#include "EmonLib.h"
#include "ota_public_key.h"

// iot004: ESP32 + W5100, Ethernet only.
const char FIRMWARE_VERSION[] = "0.1.0";
const int SENSOR_PIN = 36; // ADC1/ADC0
const int ETHERNET_CS = 5;
byte macAddress[] = {0x02, 0x00, 0x00, 0x00, 0x04, 0x01};

EnergyMonitor sensor;
EthernetClient client;
bool ethernetReady = false;

// OTA transport is intentionally not enabled until W5100 TLS is validated.
const char* OTA_SIGNING_KEY = OTA_PUBLIC_KEY_PEM;

void printNetworkInfo() {
  byte actualMac[6];
  Ethernet.MACAddress(actualMac);
  Serial.print("Firmware: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.print("MAC: ");
  for (byte index = 0; index < 6; index++) {
    if (index) Serial.print(":");
    if (actualMac[index] < 16) Serial.print("0");
    Serial.print(actualMac[index], HEX);
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Link: ");
  Serial.println(Ethernet.linkStatus() == LinkON ? "up" : "down");
}

void connectEthernet() {
  Serial.println("Solicitando endereco IP por DHCP...");
  ethernetReady = Ethernet.begin(macAddress, 5000, 1000) != 0;
  if (!ethernetReady) {
    Serial.println("Falha no DHCP.");
    return;
  }
  delay(500);
  printNetworkInfo();
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);
  analogSetPinAttenuation(SENSOR_PIN, ADC_11db);
  sensor.current(SENSOR_PIN, 6.0606);
  SPI.begin(18, 19, 23, ETHERNET_CS);
  Ethernet.init(ETHERNET_CS);
  connectEthernet();
}

void loop() {
  double currentRms = sensor.calcIrms(1480);
  Serial.print("Corrente = ");
  Serial.print(currentRms, 3);
  Serial.println(" A");

  if (!ethernetReady) {
    connectEthernet();
  }
  delay(5000);
}
