# 🐾 Pet Family Smart Feeder

> **Alimentador automático inteligente para pets** — Protótipo IoT desenvolvido com ESP32, dashboard web embarcado e monitoramento em tempo real de temperatura, umidade e nível de ração.

📄 **[Ver documentação completa](docs/index.html)**

---

## O Problema

Milhões de tutores de pets deixam seus animais sem supervisão por horas durante o dia. A ausência no horário da alimentação gera:
- Irregularidade alimentar e estresse no animal
- Dependência de terceiros para cuidados básicos
- Falta de visibilidade sobre o consumo de ração
- Impossibilidade de monitorar o ambiente do pet remotamente

---

## A Solução

O **Pet Family Smart Feeder** é um alimentador automatizado controlado por ESP32 que oferece:

- ⏱ **Alimentação automática** por intervalo configurável (minutos ou horas)
-  **Agendamento por horário fixo** (ex: todos os dias às 08:00) via NTP
-  **Dashboard web** acessível via qualquer navegador na mesma rede
-  **Monitoramento de temperatura e umidade** em tempo real (DHT22)
-  **Nível de ração** com alerta de reabastecimento
-  **Alertas sonoros** antes do fechamento automático
-  **Controle manual** via botões físicos ou interface web
-  **Log de eventos** com histórico das últimas 10 ações
-  **Atualização em tempo real** sem necessidade de recarregar a página

Tudo isso sem nenhum backend externo, banco de dados ou serviço na nuvem — o ESP32 hospeda o próprio servidor web.

---

## Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────┐
│                   ESP32 DevKit C v4                  │
│                                                     │
│  ┌──────────────┐   ┌──────────────────────────┐   │
│  │  WebServer   │   │      Estado Global        │   │
│  │  porta 80    │◄──│  (temperatura, ração,     │   │
│  │              │   │   porta, logs, uptime)    │   │
│  └──────┬───────┘   └──────────────────────────┘   │
│         │                     ▲                     │
│  ┌──────▼───────────────────────────────────────┐  │
│  │         Módulos de Controle                   │  │
│  │  feeder.cpp │ sensors.cpp │ display.cpp       │  │
│  │  alerts.cpp │ api.cpp                         │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
         │WiFi 2.4GHz
         ▼
┌─────────────────┐
│  Navegador Web  │  ← Dashboard HTML/CSS/JS
│  (qualquer      │     servido pelo próprio ESP32
│   dispositivo)  │     Atualização a cada 2s via fetch()
└─────────────────┘
```

### Fluxo da Porta (Máquina de Estados)

```
CLOSED ──trigger──► OPENING ──servo OK──► OPEN
                                           │
                               (t >= T_OPEN - T_ALARM)
                                           ▼
CLOSED ◄──servo OK── CLOSING ◄──timer──  ALARMING
   │                                        (bipe + LED)
   └── auto-feed timer / horário NTP ───────┘
```

---

## Tecnologias Utilizadas

| Camada       | Tecnologia                          |
|--------------|-------------------------------------|
| MCU          | ESP32 DevKit C v4                   |
| Firmware     | Arduino Framework (C++)             |
| Servidor Web | `WebServer.h` (embutido no ESP32)   |
| Dashboard    | HTML5 · CSS3 · JavaScript (Fetch API) |
| Sincronização de hora | NTP via `pool.ntp.org` (UTC-3) |
| Simulação    | Wokwi for VS Code                  |

---

## Sensores e Componentes

| Componente       | Pino ESP32 | Função                              |
|------------------|-----------|-------------------------------------|
| Servo SG90       | GPIO 19   | Abertura e fechamento da porta      |
| DHT22            | GPIO 18   | Temperatura e umidade do ambiente   |
| LCD I2C 16×2     | SDA 21 / SCL 22 | Display de status em tempo real |
| LED Azul         | GPIO 26   | Indica porta aberta                 |
| LED Verde        | GPIO 27   | Indica porta fechada                |
| Buzzer ativo     | GPIO 32   | Alertas sonoros e confirmações      |
| Botão ABRIR      | GPIO 33   | Disparo manual de alimentação       |
| Botão FECHAR     | GPIO 25   | Fechamento manual da porta          |

---

## Estrutura do Projeto

```
esp32-wokwi/
├── docs/
│   └── index.html             # Documentação visual do projeto
│
└── pet-smart-feeder/
    ├── pet-smart-feeder.ino   # Ponto de entrada: setup(), loop(), WiFi + NTP
    ├── config.h               # Constantes: pinos, tempos, limites
    ├── state.h                # Estrutura de estado global compartilhado
    │
    ├── feeder.cpp / .h        # Máquina de estados + servo + agendamento
    ├── sensors.cpp / .h       # Leitura periódica do DHT22
    ├── display.cpp / .h       # Gerenciamento do LCD I2C (sem flickering)
    ├── alerts.cpp / .h        # LEDs + bipe não bloqueante (sem delay)
    ├── api.cpp / .h           # Rotas HTTP + JSON + HTML do dashboard
    │
    ├── diagram.json           # Circuito Wokwi (todos os componentes)
    └── wokwi.toml             # Configuração da simulação
```

---

## API REST

O ESP32 expõe uma API REST simples:

| Método | Rota                          | Descrição                                      |
|--------|-------------------------------|------------------------------------------------|
| GET    | `/`                           | Serve o dashboard HTML completo                |
| GET    | `/api/status`                 | Retorna estado completo em JSON                |
| POST   | `/api/feed`                   | Dispara alimentação manual                     |
| POST   | `/api/close`                  | Fecha a porta manualmente                      |
| POST   | `/api/refill`                 | Reabastece ração (reseta para 100%)            |
| POST   | `/api/schedule?mode=interval&minutes=30` | Define intervalo de 30 min       |
| POST   | `/api/schedule?mode=time&hour=8&min=0`   | Alimenta todo dia às 08:00       |

### Exemplo de resposta `/api/status`

```json
{
  "doorState": "CLOSED",
  "doorOpen": false,
  "autoFeedEnabled": true,
  "useSchedule": false,
  "feedIntervalMs": 1800000,
  "schedHour": 8,
  "schedMin": 0,
  "nextFeedIn": 14800,
  "lastFeedTime": "14:32:05",
  "feedCount": 3,
  "foodLevel": 0,
  "temperature": 24.5,
  "humidity": 65.2,
  "uptime": 3612,
  "currentTime": "14:32:20",
  "logs": [
    { "time": "14:32:05", "msg": "AVISO: nivel de racao baixo - reabastecer!" },
    { "time": "14:32:00", "msg": "Alarme: encerrando ciclo de alimentacao" },
    { "time": "14:31:45", "msg": "Alimentacao automatica agendada" }
  ]
}
```

---

## Como Executar no Wokwi

### Pré-requisitos

- [VS Code](https://code.visualstudio.com/)
- Extensão [Wokwi for VS Code](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode)
- [Arduino IDE 2.x](https://www.arduino.cc/en/software) (para compilação)
- Suporte à placa ESP32 (`esp32 by Espressif Systems` no Boards Manager)

### Bibliotecas necessárias (Arduino IDE → Library Manager)

| Biblioteca              | Versão recomendada |
|-------------------------|--------------------|
| `ESP32Servo`            | >= 0.13.0          |
| `LiquidCrystal I2C`     | >= 1.1.2           |
| `DHT sensor library for ESPx (DHTesp)` | >= 1.19.0 |

### Passos

1. Abra a pasta `pet-smart-feeder/` no Arduino IDE
2. Selecione a placa: **ESP32 Dev Module**
3. Compile (`Ctrl+R`) e aguarde a geração dos binários em `build/`
4. Abra o arquivo `diagram.json` no VS Code com a extensão Wokwi
5. Pressione `F1` → `Wokwi: Start Simulator`
6. Acesse o dashboard em: **http://localhost:8280**

---

## Dashboard Web

O dashboard é servido diretamente pelo ESP32 — sem CDN, sem backend, sem dependências externas.

**Funcionalidades:**
- Status da porta em tempo real com badge colorido
- Cards de temperatura e umidade (DHT22)
- Barra de progresso animada do nível de ração
- Countdown para a próxima alimentação automática
- **Seção de Agendamento** — alterna entre modo intervalo e horário fixo
- Botão "Alimentar Agora" com cooldown de 3s
- Botão "Reabastecer Ração" (reseta para 100%)
- Log de eventos com timestamp
- Indicador de conectividade (dot verde/vermelho)
- Uptime do sistema

**Atualização:** `fetch()` a cada 2 segundos — sem recarregamento de página.

---

## Diferenciais Competitivos

| Aspecto          | Tutorial comum           | Pet Family Smart Feeder      |
|------------------|--------------------------|-------------------------------|
| Loop principal   | Bloqueante (`delay()`)   | 100% não bloqueante           |
| Dashboard        | Meta-refresh básico      | Fetch API + dark UI moderna   |
| Sensores         | Apenas servo + LED       | DHT22 + nível de ração        |
| Arquitetura      | Arquivo único monolítico | Modular com separação clara   |
| API              | Redirects HTTP           | REST com JSON padronizado     |
| Agendamento      | Intervalo fixo no código | Intervalo OU horário via web  |
| Feedback sonoro  | `tone()` bloqueante      | Bipe intermitente não bloqueante |
| LCD              | `lcd.clear()` a todo ciclo | Smart diff — sem flickering  |

---

## Futuras Melhorias

- [ ] Sensor ultrassônico HC-SR04 para medição real do nível de ração
- [ ] Múltiplos horários de alimentação por dia
- [ ] Múltiplos pets / múltiplos alimentadores em rede
- [ ] Integração com MQTT para notificações push
- [ ] Câmera ESP32-CAM para visualização do pet
- [ ] Histórico de alimentação persistido em SPIFFS
- [ ] Aplicativo mobile (React Native) consumindo a API REST

---

## Equipe...

- Pedro Vaz	RM 566551	
- João Victor Luiz Oliveira Resende	RM 565139

---

