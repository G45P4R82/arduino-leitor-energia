# Operacao

## Verificar placas

```bash
arduino-cli board list
```

As placas Uno normalmente aparecem como `/dev/ttyACM0` e `/dev/ttyACM1`. A porta pode mudar quando uma placa e conectada ou removida.

## Diagnostico Ethernet

Na serial a `9600 baud`, o firmware deve mostrar:

```text
Solicitando endereco IP por DHCP...
IP: 192.168.x.x
Dados enviados. Latencia: ... ms
```

Se aparecer `Falha no DHCP`, os LEDs do shield podem continuar piscando: eles confirmam apenas o link fisico, nao a concessao de IP.

Se houver IP mas aparecer `Falha ao conectar ao ThingSpeak`, verifique gateway, DNS, rota para a Internet e regras da rede.

## ThingSpeak

Cada canal deve ter os oito Fields habilitados com os nomes documentados no README. O envio ocorre a cada 30 segundos para respeitar o limite do ThingSpeak.

## Dados locais

`dados_sct013.csv` e arquivos de build sao artefatos locais e nao devem ser versionados.
