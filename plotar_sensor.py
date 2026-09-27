import csv
from datetime import datetime
import re
from collections import deque
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import serial


BAUDRATE = 9600
MAX_PONTOS = 60
ARQUIVO_CSV = Path(__file__).with_name("dados_sct013.csv")
PORTAS = {
    "Arduino Uno (/dev/ttyACM0)": "/dev/ttyACM0",
    "Arduino via FT232 (/dev/ttyUSB0)": "/dev/ttyUSB0",
}
CORES = ["tab:blue", "tab:orange"]

dados = {
    nome: {
        "serial": serial.Serial(porta, BAUDRATE, timeout=0.05),
        "correntes": deque(maxlen=MAX_PONTOS),
        "potencias": deque(maxlen=MAX_PONTOS),
        "tempos": deque(maxlen=MAX_PONTOS),
        "tempo": 0,
        "corrente_pendente": None,
    }
    for nome, porta in PORTAS.items()
}

arquivo_novo = not ARQUIVO_CSV.exists() or ARQUIVO_CSV.stat().st_size == 0
arquivo_csv = ARQUIVO_CSV.open("a", newline="", encoding="utf-8")
csv_writer = csv.writer(arquivo_csv)
if arquivo_novo:
    csv_writer.writerow(["data_hora", "placa", "porta", "corrente_A", "potencia_aparente_VA"])
    arquivo_csv.flush()

figura, (grafico_corrente, grafico_potencia) = plt.subplots(2, 1)
figura.suptitle("Monitor SCT-013 - duas placas")


def atualizar(_):
    for nome, dados_placa in dados.items():
        porta = dados_placa["serial"]
        while porta.in_waiting:
            linha = porta.readline().decode("ascii", errors="ignore").strip()

            corrente = re.search(r"Corrente\s*=\s*([0-9.]+)", linha)
            potencia = re.search(r"Potencia aparente\s*=\s*([0-9.]+)", linha)

            if corrente:
                dados_placa["corrente_pendente"] = float(corrente.group(1))
                dados_placa["correntes"].append(dados_placa["corrente_pendente"])
                dados_placa["tempos"].append(dados_placa["tempo"])
                dados_placa["tempo"] += 1

            if potencia:
                potencia_atual = float(potencia.group(1))
                dados_placa["potencias"].append(potencia_atual)
                if dados_placa["corrente_pendente"] is not None:
                    csv_writer.writerow(
                        [
                            datetime.now().isoformat(timespec="seconds"),
                            nome,
                            PORTAS[nome],
                            f"{dados_placa['corrente_pendente']:.3f}",
                            f"{potencia_atual:.1f}",
                        ]
                    )
                    arquivo_csv.flush()
                    dados_placa["corrente_pendente"] = None

    grafico_corrente.clear()
    grafico_potencia.clear()

    for (nome, dados_placa), cor in zip(dados.items(), CORES):
        grafico_corrente.plot(
            list(dados_placa["tempos"]),
            list(dados_placa["correntes"]),
            color=cor,
            label=nome,
        )
        grafico_potencia.plot(
            list(dados_placa["tempos"])[-len(dados_placa["potencias"]):],
            list(dados_placa["potencias"]),
            color=cor,
            label=nome,
        )

    grafico_corrente.set_ylabel("Corrente (A)")
    grafico_corrente.grid(True)
    grafico_corrente.legend(loc="upper left")
    grafico_potencia.set_ylabel("Potencia (VA)")
    grafico_potencia.set_xlabel("Amostra")
    grafico_potencia.grid(True)
    grafico_potencia.legend(loc="upper left")

    figura.tight_layout()


try:
    animacao = FuncAnimation(figura, atualizar, interval=100, cache_frame_data=False)
    plt.show()
finally:
    for dados_placa in dados.values():
        dados_placa["serial"].close()
    arquivo_csv.close()
