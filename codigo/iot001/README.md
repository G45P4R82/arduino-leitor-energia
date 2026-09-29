# iot001 com dupla publicacao

Esta versao publica a mesma telemetria em dois canais ThingSpeak:

- Canal pessoal: `1377479`
- Canal do IC: `3504146`

Os oito Fields sao enviados com o mesmo significado nos dois canais:

```text
Field 1: corrente RMS
Field 2: uptime
Field 3: latencia
Field 4: bytes enviados
Field 5: taxa de saida
Field 6: envios OK
Field 7: falhas/timeouts
Field 8: link Ethernet
```

O firmware faz duas requisicoes por ciclo de 30 segundos. As estatisticas de sucesso, falha, bytes e latencia sao mantidas separadamente para cada destino.

As chaves ficam no `secrets.h` local desta pasta, fora do Git.

## Compilacao

```bash
arduino-cli compile --fqbn arduino:avr:uno codigo/iot001
```

Ainda nao gravar sem revisar o codigo e confirmar os Fields do canal do IC.
