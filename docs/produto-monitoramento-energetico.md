# Produto: Monitoramento Energetico

## Proposta

Uma plataforma para monitorar corrente e comportamento de equipamentos eletricos usando sensores SCT-013, dispositivos IoT e um painel com historico e alertas.

## Beneficios para empresas

- Identificar equipamentos ligados fora do horario;
- Detectar consumo anormal de motores, bombas e compressores;
- Acompanhar o tempo de funcionamento de cada maquina;
- Encontrar cargas em standby;
- Comparar consumo entre maquinas, setores e unidades;
- Detectar sinais de degradacao antes de uma falha;
- Apoiar manutencao preventiva;
- Validar reducoes de consumo;
- Monitorar instalacoes remotamente;
- Gerar historico para auditorias e relatorios ESG.

## Exemplos de uso

Uma furadeira, impressora 3D ou motor pode apresentar uma corrente normal de aproximadamente `2 A`. Uma alteracao persistente para `3 A` pode indicar sobrecarga, travamento, desgaste mecanico ou alteracao no processo.

O sistema tambem pode identificar que uma maquina permanece ligada por 18 horas por dia quando deveria operar somente 8 horas.

## Componentes do produto

- Sensor SCT-013 instalado sem interromper o circuito;
- Dispositivo IoT por equipamento ou circuito;
- Comunicacao Ethernet ou Wi-Fi;
- Dashboard web;
- Historico por maquina;
- Alertas por e-mail, WhatsApp ou Telegram;
- Exportacao CSV;
- Atualizacao OTA;
- Relatorios por periodo e setor.

## Estado atual

O prototipo mede corrente RMS e calcula uma estimativa de potencia aparente:

```text
VA = corrente RMS x tensao configurada
```

Essa medicao nao representa necessariamente a potencia ativa em watts. Para medir energia real em `kWh` e calcular custo, sera necessario medir tambem a tensao instantanea e o fator de potencia.

## Evolucao tecnica

1. Medicao de corrente por equipamento;
2. Monitoramento de uptime e comunicacao;
3. Alertas por limite de corrente;
4. Medicao de tensao e potencia ativa;
5. Calculo de energia em kWh;
6. Estimativa de custo em reais;
7. OTA seguro;
8. Gestao de multiplos clientes e unidades.

## Modelo comercial

Possibilidades de receita:

- Venda ou aluguel do dispositivo;
- Instalacao do sensor;
- Mensalidade por equipamento monitorado;
- Dashboard e alertas por assinatura;
- Relatorios de eficiencia energetica;
- Planos de manutencao e suporte.

## Diferenciais

- Instalacao nao invasiva;
- Monitoramento por maquina, e nao somente por predio;
- Baixo custo inicial;
- Comunicacao Ethernet para ambientes industriais;
- Historico e indicadores de rede;
- Possibilidade de atualizacao remota;
- Arquitetura preparada para multiplos dispositivos e canais.

## Cuidados

- O SCT-013 deve envolver somente um condutor;
- A instalacao deve respeitar isolamento e seguranca eletrica;
- O circuito analogico deve respeitar o limite de tensao do ADC;
- A calibracao deve ser comparada com instrumento de referencia;
- Dados e credenciais devem ser protegidos;
- Potencia aparente nao deve ser apresentada como potencia ativa sem medir fator de potencia.
