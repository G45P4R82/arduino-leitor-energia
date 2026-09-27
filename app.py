import re
from collections import deque

import serial
from serial.tools import list_ports

from kivy.app import App
from kivy.clock import Clock
from kivy.graphics import Color, Line, Rectangle
from kivy.metrics import dp
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.uix.spinner import Spinner
from kivy.uix.widget import Widget


MAX_PONTOS = 120
CORRENTE_RE = re.compile(r"Corrente\s*=\s*([-+]?\d+(?:[.,]\d+)?)")


def velocidade_padrao(porta):
    if porta.vid == 0x2341:  # Arduino Uno
        return 9600
    if porta.vid == 0x1A86:  # ESP32/Wemos via CH340
        return 115200
    return 9600


class GraficoCorrente(Widget):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.valores = deque(maxlen=MAX_PONTOS)
        self.bind(pos=self.redesenhar, size=self.redesenhar)

    def adicionar(self, valor):
        self.valores.append(valor)
        self.redesenhar()

    def redesenhar(self, *_):
        self.canvas.clear()
        with self.canvas:
            Color(0.06, 0.08, 0.12, 1)
            Rectangle(pos=self.pos, size=self.size)

            Color(0.22, 0.27, 0.34, 1)
            for indice in range(1, 5):
                y = self.y + self.height * indice / 5
                Line(points=[self.x, y, self.right, y], width=1)

            if len(self.valores) < 2:
                return

            minimo = min(self.valores)
            maximo = max(self.valores)
            margem = max(0.1, (maximo - minimo) * 0.15)
            minimo = max(0, minimo - margem)
            maximo += margem
            escala_y = self.height / (maximo - minimo)
            escala_x = self.width / (len(self.valores) - 1)
            pontos = []

            for indice, valor in enumerate(self.valores):
                x = self.x + indice * escala_x
                y = self.y + (valor - minimo) * escala_y
                pontos.extend((x, y))

            Color(0.15, 0.75, 0.95, 1)
            Line(points=pontos, width=2.2)


class MonitorApp(App):
    title = "Monitor SCT-013"

    def build(self):
        self.portas = {}
        self.porta_serial = None
        self.evento_leitura = None

        raiz = BoxLayout(orientation="vertical", padding=dp(12), spacing=dp(10))

        barra = BoxLayout(size_hint_y=None, height=dp(48), spacing=dp(8))
        self.seletor = Spinner(text="Selecione a porta", values=())
        barra.add_widget(self.seletor)

        atualizar = Button(text="Atualizar", size_hint_x=None, width=dp(100))
        atualizar.bind(on_press=lambda *_: self.atualizar_portas())
        barra.add_widget(atualizar)

        self.botao_conectar = Button(text="Conectar", size_hint_x=None, width=dp(100))
        self.botao_conectar.bind(on_press=lambda *_: self.alternar_conexao())
        barra.add_widget(self.botao_conectar)
        raiz.add_widget(barra)

        self.status = Label(
            text="Selecione uma porta serial",
            size_hint_y=None,
            height=dp(28),
            halign="left",
            valign="middle",
        )
        self.status.bind(size=lambda widget, value: setattr(widget, "text_size", value))
        raiz.add_widget(self.status)

        self.valor_atual = Label(
            text="-- A",
            font_size="28sp",
            size_hint_y=None,
            height=dp(55),
        )
        raiz.add_widget(self.valor_atual)

        self.grafico = GraficoCorrente()
        raiz.add_widget(self.grafico)
        self.atualizar_portas()
        return raiz

    def atualizar_portas(self):
        self.portas = {}
        nomes = []
        for porta in list_ports.comports():
            baudrate = velocidade_padrao(porta)
            descricao = porta.description or "dispositivo serial"
            nome = f"{porta.device} - {descricao} ({baudrate})"
            self.portas[nome] = (porta.device, baudrate)
            nomes.append(nome)

        self.seletor.values = nomes
        if nomes and self.seletor.text not in self.portas:
            self.seletor.text = nomes[0]
        elif not nomes:
            self.seletor.text = "Nenhuma porta encontrada"
        self.status.text = f"{len(nomes)} porta(s) encontrada(s)"

    def alternar_conexao(self):
        if self.porta_serial is not None:
            self.desconectar()
            return

        configuracao = self.portas.get(self.seletor.text)
        if configuracao is None:
            self.status.text = "Selecione uma porta válida"
            return

        dispositivo, baudrate = configuracao
        try:
            self.porta_serial = serial.Serial(dispositivo, baudrate, timeout=0.05)
        except serial.SerialException as erro:
            self.status.text = f"Erro ao abrir {dispositivo}: {erro}"
            return

        self.grafico.valores.clear()
        self.grafico.redesenhar()
        self.botao_conectar.text = "Desconectar"
        self.status.text = f"Conectado em {dispositivo} a {baudrate} baud"
        self.evento_leitura = Clock.schedule_interval(self.ler_serial, 0.1)

    def desconectar(self):
        if self.evento_leitura is not None:
            self.evento_leitura.cancel()
            self.evento_leitura = None
        if self.porta_serial is not None:
            self.porta_serial.close()
            self.porta_serial = None
        self.botao_conectar.text = "Conectar"
        self.status.text = "Desconectado"

    def ler_serial(self, _):
        if self.porta_serial is None:
            return

        while self.porta_serial.in_waiting:
            texto = self.porta_serial.readline().decode("ascii", errors="ignore").strip()
            resultado = CORRENTE_RE.search(texto)
            if resultado:
                valor = float(resultado.group(1).replace(",", "."))
                self.grafico.adicionar(valor)
                self.valor_atual.text = f"{valor:.3f} A"

    def on_stop(self):
        self.desconectar()


if __name__ == "__main__":
    MonitorApp().run()
