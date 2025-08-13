import requests
from datetime import datetime
import numpy as np

# Simulação dos dados do sensor ESP32
agora = datetime.now()
hora_dia_min = agora.hour * 60 + agora.minute

dados_sensor = {
    "location": "ESP_SIMULADO_01",
    "datetime": agora.isoformat(),
    "chuva": 80.0,                # <-- Adicione este campo!
    "temperatura": 26.5,
    "umidade_solo": 48.0,
    "luminosidade": 1200.0,
    "nivel_agua": 520.0,
    "vibracao_solo": 320.0,
    "tempo_minutos": (agora.day - 1) * 24 * 60 + agora.hour * 60 + agora.minute,
    "hora_sin": float(np.sin(2 * np.pi * hora_dia_min / 1440)),
    "hora_cos": float(np.cos(2 * np.pi * hora_dia_min / 1440)),
}

# Envia para a API FastAPI local
res = requests.post("http://localhost:8000/sensor", json=dados_sensor)

print("Resposta:", res.status_code)
print(res.json())

'''import json
import requests

# Simulação dos dados do sensor ESP32 em JSON

with open("dados_sensor.json", "r", encoding="utf-8") as f:
    dados_sensor = json.load(f)

res = requests.post("http://localhost:8000/sensor", json=dados_sensor)
print("Resposta:", res.status_code)
print(res.json())'''