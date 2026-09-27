# Arduino Leitor de Energia

Monitoramento de corrente AC com sensores SCT-013, Arduino Uno, shields Ethernet e ThingSpeak.

## Visao geral

O projeto possui dois dispositivos independentes:

- `iot001`: canal ThingSpeak `1377479`.
- `iot002`: canal ThingSpeak `3510600`.

Cada dispositivo mede corrente RMS no pino `A0` e publica telemetria a cada 30 segundos.

## Telemetria

| Field | Dado |
| --- | --- |
| 1 | Corrente (A) |
| 2 | Uptime (s) |
| 3 | Latencia (ms) |
| 4 | Bytes enviados |
| 5 | Taxa de saida (B/s) |
| 6 | Envios OK |
| 7 | Falhas/timeouts |
| 8 | Link Ethernet (0/1) |

O IP local e enviado no campo textual `status` do ThingSpeak.

## Hardware

- Arduino Uno
- Ethernet Shield W5100 ou W5500
- SCT-013-000
- Resistor de carga de 330 ohms
- Circuito de offset conforme a montagem do sensor
- Saida do sensor no `A0`
- Cabo Ethernet conectado a uma rede com acesso ao ThingSpeak

O sensor deve envolver somente um condutor da carga. Nunca envolva fase e neutro juntos.

## Calibracao

- Sensor: SCT-013-000
- Resistor de carga: 330 ohms
- Calibracao EmonLib: `6.0606`
- Tensao usada na estimativa local: `127 V`
- Amostras RMS: `1480`

A potencia aparente exibida na serial e uma estimativa. Potencia real exige medicao simultanea da tensao e fator de potencia.

## Firmware

Os sketches versionados ficam em `codigo/`:

- `codigo/iot001.ino`
- `codigo/iot002.ino`

O arquivo `arduinho-leitor-energia.ino` e o sketch ativo usado para gravar a placa conectada no momento.

### Configuracao de chaves

As chaves nao ficam no Git. Copie o modelo:

```bash
cp secrets.h.example secrets.h
```

Edite `secrets.h` e substitua os valores. O arquivo e ignorado pelo Git.

### Compilar e gravar

Instale o Arduino CLI e os cores/bibliotecas necessários. Para um Uno conectado em `/dev/ttyACM0`:

```bash
arduino-cli compile --fqbn arduino:avr:uno .
arduino-cli upload . --fqbn arduino:avr:uno --port /dev/ttyACM0 --verify
```

O shield usa o pino CS `10` e a serial opera em `9600 baud`.

## Monitor ThingSpeak

O `monitor_serial.py` e uma interface Kivy que nao usa USB. Ele consulta a API privada, permite selecionar `iot001` ou `iot002`, exibe os metadados do canal, os oito Fields, logs e grafico da corrente.

Crie e ative o ambiente virtual:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
cp .env.example .env
```

Preencha as chaves de leitura em `.env` e execute:

```bash
python monitor_serial.py
```

O `app.py` e o monitor local por USB. Ele lista as portas, conecta ao Uno/ESP32 e mostra a corrente em tempo real.

## Seguranca

Nao publique Write API Keys, Read API Keys, senhas Wi-Fi ou arquivos `.env`/`secrets.h`. Como chaves foram compartilhadas durante o desenvolvimento, gere novas chaves no ThingSpeak antes de usar este repositorio fora do ambiente local.
