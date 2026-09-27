from collections import deque
import os
from pathlib import Path
from threading import Lock, Thread

import requests
from kivy.app import App
from kivy.clock import Clock
from kivy.graphics import Color, Line, Rectangle
from kivy.metrics import dp
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.spinner import Spinner
from kivy.uix.textinput import TextInput
from kivy.uix.widget import Widget


POLL_INTERVAL = 10
MAX_PONTOS = 120
FIELD_LABELS = {
    1: "Corrente (A)",
    2: "Uptime (s)",
    3: "Latencia (ms)",
    4: "Bytes enviados",
    5: "Taxa de saida (B/s)",
    6: "Envios OK",
    7: "Falhas/timeouts",
    8: "Link Ethernet",
}


def carregar_env_local():
    arquivo = Path(__file__).with_name(".env")
    if not arquivo.exists():
        return
    for linha in arquivo.read_text(encoding="utf-8").splitlines():
        linha = linha.strip()
        if linha and not linha.startswith("#") and "=" in linha:
            chave, valor = linha.split("=", 1)
            os.environ.setdefault(chave.strip(), valor.strip())


carregar_env_local()
CANAIS = {
    "iot001 - canal 1377479": {
        "id": 1377479,
        "read_key": os.getenv("THINGSPEAK_IOT001_READ_KEY", ""),
    },
    "iot002 - canal 3510600": {
        "id": 3510600,
        "read_key": os.getenv("THINGSPEAK_IOT002_READ_KEY", ""),
    },
}


class Grafico(Widget):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.valores = deque(maxlen=MAX_PONTOS)
        self.bind(pos=self.desenhar, size=self.desenhar)

    def adicionar(self, valor):
        self.valores.append(valor)
        self.desenhar()

    def desenhar(self, *_):
        self.canvas.clear()
        with self.canvas:
            Color(0.05, 0.07, 0.10, 1)
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
            pontos = []
            for indice, valor in enumerate(self.valores):
                x = self.x + self.width * indice / (len(self.valores) - 1)
                y = self.y + self.height * (valor - minimo) / (maximo - minimo)
                pontos.extend((x, y))

            Color(0.15, 0.75, 0.95, 1)
            Line(points=pontos, width=2.2)


class ThingSpeakApp(App):
    title = "Monitor ThingSpeak"

    def build(self):
        self.entradas_vistas = set()
        self.log_linhas = deque(maxlen=25)
        self.polling = False
        self.lock = Lock()
        self.canal_nome = next(iter(CANAIS))

        raiz = BoxLayout(orientation="vertical", padding=dp(12), spacing=dp(8))

        cabecalho = BoxLayout(size_hint_y=None, height=dp(48), spacing=dp(8))
        self.seletor = Spinner(
            text=self.canal_nome,
            values=tuple(CANAIS),
            size_hint_x=None,
            width=dp(230),
        )
        self.seletor.bind(text=self.selecionar_canal)
        cabecalho.add_widget(self.seletor)
        self.status = Label(text="Iniciando")
        cabecalho.add_widget(self.status)
        atualizar = Button(text="Atualizar agora", size_hint_x=None, width=dp(130))
        atualizar.bind(on_press=lambda *_: self.consultar())
        cabecalho.add_widget(atualizar)
        raiz.add_widget(cabecalho)

        self.informacoes = Label(
            text="Channel ID: -- | Last entry: -- | Atualizado: --",
            size_hint_y=None,
            height=dp(30),
        )
        raiz.add_widget(self.informacoes)

        self.valor = Label(text="-- A", font_size="28sp", size_hint_y=None, height=dp(55))
        raiz.add_widget(self.valor)
        raiz.add_widget(Label(text="Field 1 - corrente recebida", size_hint_y=None, height=dp(25)))

        self.grafico = Grafico()
        raiz.add_widget(self.grafico)

        self.metricas = {}
        painel = GridLayout(cols=2, spacing=dp(4), size_hint_y=None, height=dp(120))
        for indice, nome in FIELD_LABELS.items():
            label = Label(text=f"{nome}: --", halign="left")
            label.bind(size=lambda widget, value: setattr(widget, "text_size", value))
            self.metricas[indice] = label
            painel.add_widget(label)
        raiz.add_widget(painel)

        raiz.add_widget(Label(text="Logs do ThingSpeak", size_hint_y=None, height=dp(25)))
        self.log = TextInput(readonly=True, multiline=True, size_hint_y=None, height=dp(165))
        raiz.add_widget(self.log)

        Clock.schedule_interval(lambda *_: self.consultar(), POLL_INTERVAL)
        Clock.schedule_once(lambda *_: self.consultar(), 0.2)
        return raiz

    def consultar(self):
        with self.lock:
            if self.polling:
                return
            self.polling = True
        canal_nome = self.canal_nome
        Thread(target=self.consultar_em_thread, args=(canal_nome,), daemon=True).start()

    def consultar_em_thread(self, canal_nome):
        canal = CANAIS[canal_nome]
        url = (
            f"https://api.thingspeak.com/channels/{canal['id']}/feeds.json"
            f"?api_key={canal['read_key']}&results=25"
        )
        try:
            resposta = requests.get(url, timeout=8)
            resposta.raise_for_status()
            dados = resposta.json()
            Clock.schedule_once(lambda _: self.processar_feeds(canal_nome, dados), 0)
        except requests.RequestException as erro:
            Clock.schedule_once(lambda _: self.registrar(f"ERRO de rede: {erro}"), 0)
        finally:
            with self.lock:
                self.polling = False

    def selecionar_canal(self, _, canal_nome):
        if canal_nome not in CANAIS or canal_nome == self.canal_nome:
            return
        self.canal_nome = canal_nome
        self.entradas_vistas.clear()
        self.log_linhas.clear()
        self.log.text = ""
        self.grafico.valores.clear()
        self.grafico.desenhar()
        self.valor.text = "-- A"
        self.informacoes.text = "Channel ID: -- | Last entry: -- | Atualizado: --"
        for indice, nome in FIELD_LABELS.items():
            self.metricas[indice].text = f"{nome}: --"
        self.registrar(f"Canal selecionado: {canal_nome}")
        Clock.schedule_once(lambda _: self.consultar(), 0.2)

    def processar_feeds(self, canal_nome, dados):
        if canal_nome != self.canal_nome:
            return

        canal = dados.get("channel", {})
        canal_id = canal.get("id", CANAIS[canal_nome]["id"])
        ultima_entrada = canal.get("last_entry_id") or "--"
        atualizado = canal.get("updated_at") or "--"
        self.informacoes.text = (
            f"Channel ID: {canal_id} | Last entry: {ultima_entrada} | "
            f"Atualizado: {atualizado} | Field 1: {canal.get('field1', '---')}"
        )

        feeds = dados.get("feeds", [])
        if feeds:
            ultimo = feeds[-1]
            for indice, nome in FIELD_LABELS.items():
                valor = ultimo.get(f"field{indice}")
                self.metricas[indice].text = f"{nome}: {valor if valor not in (None, '') else '--'}"

        novas = 0
        for feed in feeds:
            entry_id = feed.get("entry_id")
            if entry_id in self.entradas_vistas:
                continue
            self.entradas_vistas.add(entry_id)

            valor_texto = feed.get("field1")
            if valor_texto not in (None, ""):
                try:
                    valor = float(str(valor_texto).replace(",", "."))
                    self.grafico.adicionar(valor)
                    self.valor.text = f"{valor:.3f} A"
                    campos = " | ".join(
                        f"F{indice}={feed.get(f'field{indice}', '--')}"
                        for indice in range(1, 9)
                        if feed.get(f"field{indice}") not in (None, "")
                    )
                    self.registrar(f"Entry {entry_id} | {feed.get('created_at')} | {campos}")
                except ValueError:
                    self.registrar(f"Entry {entry_id} | Field 1 inválido: {valor_texto}")
            else:
                self.registrar(f"Entry {entry_id} | Field 1 vazio")
            novas += 1

        self.status.text = f"{canal_nome}: {len(feeds)} entradas consultadas"
        if novas == 0:
            self.registrar("Consulta concluída: nenhum dado novo")

    def registrar(self, mensagem):
        self.log_linhas.append(mensagem)
        self.log.text = "\n".join(self.log_linhas)
        self.log.cursor = (0, len(self.log.text.splitlines()))

    def on_stop(self):
        self.polling = False


if __name__ == "__main__":
    ThingSpeakApp().run()
