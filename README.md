# LifePet - Monitor Ambiental IoT

Disruptive Architectures: IoT, IoB & Generative IA

---

## Problema

A jornada de saúde do pet é fragmentada. O tutor só lembra de cuidar do animal quando surge um problema visível, esquecendo fatores ambientais que afetam o bem-estar do pet no dia a dia, como temperatura e umidade do ambiente onde ele vive.

## Solução

O LifePet é um app de agenda inteligente para saúde do pet. Este repositório contém o módulo IoT: um dispositivo ESP32 com sensor DHT22 que monitora o ambiente do animal e expõe os dados em uma API REST HTTP, consumida por um dashboard em tempo real.

O sistema combina dois tipos de dado:
- Leitura local do sensor DHT22 (temperatura e umidade do ambiente do pet)
- Dados externos da API Open-Meteo (temperatura e umidade da cidade de São Paulo)

O dashboard compara os dois valores e indica se o ambiente interno está mais quente, mais frio ou igual ao externo. Quando a temperatura sai da faixa de conforto (18C a 26C), o LED vermelho acende e o dashboard exibe alerta.

---

## Tecnologias

- Microcontrolador: ESP32 DevKit C v4
- Sensor: DHT22 (temperatura e umidade)
- Display: LCD 1602 I2C
- Protocolo: HTTP REST
- API externa: Open-Meteo (dados climáticos de São Paulo)
- Simulador: Wokwi
- Dashboard: HTML + JavaScript

---

## Endpoints da API

| Metodo | Rota | Descricao |
|--------|------|-----------|
| GET | /api/status | Leitura local + dados externos em um JSON |
| GET | /api/historico | Ultimas 10 leituras do sensor local |

Exemplo de resposta do /api/status:

```json
{
  "local": {
    "temperatura": 24.3,
    "umidade": 58.0,
    "confortavel": true,
    "temp_min": 18.0,
    "temp_max": 26.0
  },
  "externo": {
    "temperatura": 21.5,
    "umidade": 72.0,
    "disponivel": true
  }
}
```

---

## Estrutura do projeto

```
lifepet-iot/
├── lifepet.ino      - codigo do ESP32
├── diagram.json     - circuito no Wokwi
├── wokwi.toml       - configuração do simulador
├── dashboard.html   - dashboard web
└── README.md        - arquivo de leitura
```

---

## Como executar

**1. Compilar**

Abra o lifepet.ino no Arduino IDE, selecione a placa DOIT ESP32 DEVKIT V1 e va em Sketch > Exportar Binario Compilado. Isso gera a pasta build/.

Bibliotecas necessarias: DHTesp, LiquidCrystal I2C, ArduinoJson.

**2. Simular**

Abra a pasta no VS Code, abra o diagram.json e inicie a simulacao pelo Wokwi. No terminal aguarde a mensagem "Servidor HTTP iniciado". O ESP32 vai buscar os dados externos automaticamente logo apos conectar ao WiFi.

**3. Dashboard**

Abra o dashboard.html no navegador. Ele aponta para localhost:8280, que o wokwi.toml redireciona para o ESP32 simulado. Os dados locais atualizam a cada 5 segundos e os externos a cada 30 segundos.

---

## Como o ESP32 se encaixa no LifePet

O app LifePet tem modulos de vacinas, consultas e medicamentos — dados que dependem da acao do tutor. O IoT traz monitoramento passivo e continuo: dados que chegam sem o tutor precisar fazer nada. O comparativo com o clima externo permite alertas inteligentes como "o ambiente do seu pet esta 6C mais quente que o lado de fora — considere ligar o ar condicionado".

---

## Resultados parciais

- Leitura de temperatura e umidade local a cada 5 segundos
- Consulta a API externa Open-Meteo a cada 30 segundos
- API REST com dois endpoints funcionais
- Dashboard com alertas visuais e comparativo interno vs externo
- LCD exibindo temperatura local e externa simultaneamente
- Temporizacao nao-bloqueante com millis()

---

## Equipe da LifePet

Aguinel Junior - rm564857

Felipe da Silva - rm563485

Henrique Gonçalves - rm562086

Leonardo Saavedra - rm562229

Vitor Mendes - rm565376
