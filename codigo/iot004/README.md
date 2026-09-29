# iot004 - ESP32 + W5100 com OTA

Firmware experimental para ESP32 conectado por Ethernet ao shield W5100.

## Hardware

- ESP32
- W5100
- Conversor de nivel logico 5 V/3,3 V, obrigatorio quando o shield nao possui adaptacao
- SCT-013 no GPIO36/ADC1

## Build

O firmware usa a particao OTA padrao do ESP32. O primeiro upload e feito pela USB; as atualizacoes futuras serao publicadas por GitHub Release.

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=default codigo/iot004
```

## Release

Tags no formato `iot004-vX.Y.Z` acionam o workflow de release. O pacote publicado contem o binario, manifesto, SHA-256 e assinatura.

O cliente OTA seguro sera habilitado depois da validacao eletrica e do teste DHCP do W5100.
