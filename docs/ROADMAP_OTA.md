# Roadmap de OTA via Ethernet

## Motivacao

Os `iot001` e `iot002` usam Arduino Uno com Ethernet Shield. O Uno possui somente 32 KB de Flash e 2 KB de SRAM, nao possui particoes OTA e nao oferece rollback seguro.

O firmware atual utiliza aproximadamente 21 KB de Flash. Implementar OTA diretamente no Uno exigiria um bootloader Ethernet customizado, area para gravacao e um mecanismo de recuperacao. Uma falha durante a atualizacao poderia inutilizar a placa.

## Arquitetura recomendada

Migrar futuras unidades para:

```text
ESP32 + W5500 Ethernet
```

O ESP32 operara exclusivamente pela Ethernet cabeada, sem depender do Wi-Fi. O W5500 deve ser compativel com logica de 3,3 V ou usar conversao de nivel adequada.

Vantagens:

- Flash e RAM suficientes para OTA;
- Particoes OTA;
- Rollback automatico;
- HTTPS;
- Verificacao de hash e assinatura;
- Watchdog;
- Integracao simples com GitHub Actions;
- Hardware acessivel.

## Alternativas

### Arduino Opta

Plataforma industrial com Ethernet e mais memoria. Pode usar atualizacao remota pelo ecossistema Arduino Cloud, com custo maior.

### Portenta H7 com Ethernet Carrier

Solucao de alto desempenho e adequada para ambientes industriais, mas com custo significativamente maior.

### Arduino Due ou Mega

Possuem mais memoria que o Uno, mas nao possuem OTA nativo nem rollback seguro. Nao sao a primeira escolha para este roadmap.

## Fluxo CI/CD proposto

```text
Commit ou tag no GitHub
        |
        v
GitHub Actions compila o firmware
        |
        v
Gera binario, SHA-256 e assinatura
        |
        v
Publica em GitHub Release ou VPS HTTPS
        |
        v
ESP32 consulta manifest.json
        |
        v
Valida versao, hash e assinatura
        |
        v
Instala OTA e reinicia
        |
        v
Confirma funcionamento ou executa rollback
```

## Manifesto de firmware

Exemplo de arquivo publicado no servidor:

```json
{
  "device": "iot003",
  "version": "1.2.0",
  "url": "https://servidor.exemplo/firmware/iot003/1.2.0.bin",
  "sha256": "hash-do-binario",
  "signature": "assinatura-digital"
}
```

## Requisitos de seguranca

- Download somente via HTTPS;
- Validacao do certificado ou CA;
- Validacao de SHA-256;
- Assinatura digital do firmware;
- Rollback se o boot falhar;
- Watchdog ativo;
- Releases imutaveis;
- Atualizacao gradual por grupos;
- Nenhuma Write API Key, senha Wi-Fi ou segredo no repositorio;
- Logs de versao, sucesso e falha por dispositivo.

## Plano de migracao

1. Manter `iot001` e `iot002` via USB enquanto estiverem estaveis.
2. Criar um prototipo ESP32 + W5500.
3. Migrar a leitura do SCT-013 para ADC1 e nivel de 3,3 V.
4. Implementar Ethernet e ThingSpeak.
5. Adicionar particoes OTA e rollback.
6. Criar GitHub Actions para compilacao e assinatura.
7. Testar em um dispositivo piloto.
8. Fazer rollout gradual para os demais dispositivos.

## Observacao

Um gateway externo poderia atualizar o Uno remotamente, mas adiciona hardware, complexidade e pontos de falha. Para uma nova revisao de hardware, ESP32 + W5500 e a alternativa preferencial.
