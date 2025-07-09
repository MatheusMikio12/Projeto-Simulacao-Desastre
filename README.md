# Sistema de Monitoramento e Previsão de Desastres Naturais com ESP32 + Machine Learning

Este projeto reúne **IoT** e **Inteligência Artificial** para construir uma solução completa de **monitoramento ambiental e previsão de desastres naturais**. A coleta de dados é feita com sensores simulados em uma placa **ESP32**, e os dados alimentam modelos de **Machine Learning** capazes de prever a ocorrência e o tipo de eventos críticos como **enchentes**, **deslizamentos** e **incêndios**.

---

## Parte 1 – Captura de Dados com ESP32

### Objetivo

Desenvolver um sistema embarcado com a placa **ESP32** e sensores ambientais simulados para monitorar áreas de risco e gerar alertas em tempo real.

### Tecnologias e Ferramentas

- ESP32 Dev Module  
- [Wokwi](https://wokwi.com/) (simulador online)  
- VS Code + PlatformIO  
- Python (captura de dados via serial)  
- CSV / Google Sheets (armazenamento dos dados)

### Sensores Simulados

| Sensor         | Finalidade                                      |
|----------------|--------------------------------------------------|
| **DHT22**       | Temperatura e umidade do ar                     |
| **LDR**         | Luminosidade (indicador de fumaça ou apagão)    |
| **Ultrassônico**| Nível da água / alagamento                      |
| **Potenciômetro** | Simula vibração do solo (deslizamentos)       |

### Lógica de Alerta

- 🟢 **LED Verde**: Situação normal  
- 🟡 **LED Amarelo**: Alerta moderado  
- 🔴 **LED Vermelho**: Alto risco de desastre

### Coleta de Dados

Os dados lidos dos sensores são enviados via serial para o computador e salvos em `.csv`, com variáveis como:

- Temperatura  
- Umidade  
- Luminosidade  
- Nível da água  
- Vibração do solo  
- Rótulo de risco (manual ou automatizado)

---

## Parte 2 – Machine Learning para Previsão de Desastres

### Objetivo

Utilizar os dados capturados pela ESP32 para treinar modelos de **classificação supervisionada** que detectem padrões de risco ambiental e **prevejam desastres naturais** antes que ocorram.

### Modelos

- **Modelo Binário**: prevê se haverá ou não um desastre (0 = seguro, 1 = risco)
- **Modelo Multiclasse**: identifica o tipo de desastre:
  - 1 = Calor/Umidade crítica
  - 2 = Incêndio / fumaça densa
  - 3 = Vibração moderada
  - 4 = Deslizamento (vibração forte)
  - 5 = Nível da água elevado
  - 6 = Enchente iminente

### Tecnologias

- Python
- Pandas / NumPy
- Scikit-learn
- Matplotlib / Seaborn
- Jupyter Notebook ou Google Colab

### Funcionalidades

- Pré-processamento com variáveis temporais (`Hora_sin`, `Hora_cos`, `Tempo_Minutos`)
- Modelos Random Forest para classificação
- Testes com novos dados simulados
- Gráficos explicativos para facilitar entendimento por leigos

---

## Integração Final

O objetivo é que os dados lidos em tempo real pelo ESP32 sejam enviados para um sistema que execute os modelos de ML, gere previsões e acione alarmes visuais ou envie alertas para a nuvem (via Wi-Fi/MQTT).

---

## Autor
Este projeto foi desenvolvido por Ian Nobres Azevedo e Matheus Mikio Tutume Lourenço como parte de estudos e aplicação prática de IoT e Machine Learning.
