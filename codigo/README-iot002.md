# iot002 - Segundo medidor de corrente Ethernet

Firmware do Arduino Uno com shield Ethernet para medir o SCT-013 no pino `A0` e publicar telemetria no ThingSpeak.

## ThingSpeak

- Canal: `3510600`
- Nome: `iot-2-amperes`
- Field 1: `Corrente (A)`
- Field 2: Uptime (s)
- Field 3: Latencia (ms)
- Field 4: Bytes enviados
- Field 5: Taxa de saida (B/s)
- Field 6: Envios OK
- Field 7: Falhas ou timeouts
- Field 8: Link Ethernet
- Intervalo de envio: 30 segundos

As chaves ficam em `secrets.h` e nao sao versionadas.

## Hardware

- Arduino Uno
- Shield Ethernet W5100/W5500
- SCT-013-000
- Resistor de carga de 330 ohms
- Saida do sensor no `A0`
- Rede Ethernet com DHCP

## Calibracao

- Fator EmonLib: `6.0606`
- Tensao local configurada: `127 V`
- Amostras RMS: `1480`

A potencia aparente aparece na serial. O firmware envia os oito indicadores a cada 30 segundos e o IP no campo textual `status`.

## Comunicacao

Serial: `9600 baud`.

O shield usa o pino CS `10`.

## Gravacao

```bash
arduino-cli compile --fqbn arduino:avr:uno .
arduino-cli upload . --fqbn arduino:avr:uno --port /dev/ttyACM0 --verify
```

O arquivo `iot002.ino` e a copia versionada deste firmware. O sketch principal do projeto tambem fica configurado para o iot002 enquanto esta placa estiver em uso.
