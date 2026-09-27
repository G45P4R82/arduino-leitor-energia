# iot001 - Medidor de corrente com Ethernet

Firmware do Arduino Uno com shield Ethernet para medir corrente AC usando o SCT-013 e publicar os dados no ThingSpeak.

## Hardware

- Arduino Uno
- Ethernet Shield baseado em W5100/W5500
- Sensor SCT-013-000
- Circuito de carga de 330 ohms
- Saida do circuito do sensor no pino analogico `A0`
- Cabo Ethernet conectado a uma rede com DHCP

## ThingSpeak

- Canal: `1377479`
- Nome: `iot - telemetria - atp`
- Field 1: `Corrente (A)`
- Field 2: Uptime (s)
- Field 3: Latencia (ms)
- Field 4: Bytes enviados
- Field 5: Taxa de saida (B/s)
- Field 6: Envios OK
- Field 7: Falhas ou timeouts
- Field 8: Link Ethernet

O firmware envia os oito indicadores a cada 30 segundos. O IP e enviado no campo textual `status`.

### Chaves

As chaves ficam em `secrets.h` e nao sao versionadas.

Endpoint de escrita usado pelo firmware:

```text
http://api.thingspeak.com/update
```

## Calibracao

- Sensor: SCT-013-000
- Resistor de carga: 330 ohms
- Fator EmonLib: `2000 / 330 = 6.0606`
- Tensao configurada para calculo local: 127 V
- Amostras RMS: 1480

A potencia aparente e mostrada apenas na serial. Ela nao e enviada ao ThingSpeak.

## Serial

```text
9600 baud
```

Mensagens esperadas:

```text
Solicitando endereco IP por DHCP...
IP: 192.168.x.x
Corrente = 1.234 A
Potencia aparente = 156.7 VA
Dados enviados ao ThingSpeak.
```

## Compilacao e gravacao

O sketch principal do projeto pode ser compilado normalmente:

```bash
arduino-cli compile --fqbn arduino:avr:uno .
arduino-cli upload . --fqbn arduino:avr:uno --port /dev/ttyACM0 --verify
```

O arquivo `iot001.ino` e a copia versionada deste firmware. Para usa-lo como sketch independente, coloque-o em uma pasta chamada `iot001` e mantenha o nome `iot001.ino`.

## Diagnostico

Se a serial parar em `Solicitando endereco IP por DHCP...`, verifique:

- Cabo Ethernet conectado;
- LEDs de link do shield;
- Rede com DHCP ativo;
- Shield corretamente encaixado;
- Se o shield usa o pino CS 10.

As chaves de API sao credenciais. Se este repositorio for compartilhado, gere novas chaves no ThingSpeak.
