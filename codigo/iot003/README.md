# iot003 - ESP32 Wi-Fi

Firmware da Wemos/ESP32 para medir o SCT-013 e publicar nos canais ThingSpeak `3511204` e `3513189`.

## Hardware

- Wemos D1 R32 com ESP32
- SCT-013
- Sinal do sensor no `IO36/GPIO36` (ADC1/ADC0)
- Circuito analógico limitado a 3,3 V

O `GPIO2` nao deve ser usado para esta leitura quando o Wi-Fi estiver ativo, pois ele pertence ao ADC2. O firmware usa diretamente o `GPIO36`.

## ThingSpeak

- Canal: `3511204`
- Canal pessoal: `3511204` (`iot-3-amperes`)
- Canal IC: `3513189` (`projeto-26-2s-C`)
- Field 1: corrente
- Field 2: uptime
- Field 3: latencia
- Field 4: bytes enviados
- Field 5: taxa de saida
- Field 6: envios OK
- Field 7: falhas/timeouts
- Field 8: link Wi-Fi

O mesmo pacote de oito Fields e enviado aos dois canais. O IP e enviado no campo `status`. O envio ocorre a cada 30 segundos.

## Rede

As credenciais ficam em `secrets.h`, que nao e versionado:

```cpp
#define WIFI_SSID "IoT-local"
#define WIFI_PASSWORD "..."
```

## Gravacao

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 codigo/iot003
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 --input-dir codigo/iot003/build/esp32.esp32.esp32
```

Serial: `115200 baud`.
