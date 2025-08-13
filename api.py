from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from datetime import datetime, date
import joblib

# ----- CONFIG FASTAPI -----
app = FastAPI()

# ----- LIBERA CORS para o Front-end -----
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Substitua por URL do seu front para mais segurança
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ----- CLASSES PARA JSON -----
class SensorData(BaseModel):
    location: str
    datetime: datetime
    chuva: float
    temperatura: float
    umidade_solo: float
    luminosidade: float
    nivel_agua: float
    vibracao_solo: float
    tempo_minutos: float
    hora_sin: float
    hora_cos: float

class EntradaPrevisao(BaseModel):
    location: str
    date: date

# ----- BANCO TEMPORÁRIO EM MEMÓRIA -----
dados_recebidos = []

# ----- CARREGAMENTO DO MODELO -----
try:
    modelo = joblib.load("modelo.pkl")
    print("✅ Modelo carregado com sucesso")
except Exception as e:
    print("❌ Erro ao carregar modelo:", e)
    modelo = None

# ----- ENDPOINT DE RECEBIMENTO DO SENSOR -----
@app.post("/sensor")
def receber_dados(dados: SensorData):
    dados_recebidos.append(dados)
    return {"mensagem": "Dados recebidos com sucesso"}

# ----- ENDPOINT DE PREVISÃO -----
@app.post("/prever")
def prever(dados: EntradaPrevisao):
    if modelo is None:
        raise HTTPException(status_code=500, detail="Modelo não carregado")

    entrada = None
    for d in dados_recebidos:
        # Converte d.datetime para objeto datetime se for string
        dt = d.datetime
        if isinstance(dt, str):
            dt = datetime.fromisoformat(dt)
        if d.location == dados.location and dt.date() == dados.date:
            entrada = d
            break

    # ...existing code...

    if not entrada:
        raise HTTPException(status_code=404, detail="Dados do sensor não encontrados para essa data/localização")

    # Corrigido: vetor X com todas as 8 features esperadas pelo modelo
    X = [[
        entrada.temperatura,      # Temperatura (ºC)
        entrada.umidade_solo,          # Umidade (%)
        entrada.luminosidade,     # Luminosidade (LUX)
        entrada.nivel_agua,       # Nivel da agua
        entrada.vibracao_solo,    # Vibração do solo
        entrada.tempo_minutos,    # Tempo_Minutos
        entrada.hora_sin,         # Hora_sin
        entrada.hora_cos          # Hora_cos
    ]]
    pred = modelo.predict(X)[0]

    # ...existing
    if pred == 1:
        return {
            "riskLevel": "Alto",
            "rainfall": f"{entrada.chuva}mm",
            "recommendation": "Evacuar a área imediatamente."
        }
    else:
        return {
            "riskLevel": "Baixo",
            "rainfall": f"{entrada.chuva}mm",
            "recommendation": "Sem necessidade de evacuação."
        }