// Inclusão das bibliotecas necessárias para o sensor DHT22
#include <wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Ultrasonic.h>
#include <SHA256.h>
 
// Definição dos pinos para os dispositivos conectados ao ESP32
#define DHT_PIN 16        // Sensor DHT22 conectado ao pino D4
#define LED_VERMELHO 23        // LED AZUL conectado ao pino D2
#define LED_AMARELO 22        // LED VERMELHO conectado ao pino D2
#define LED_VERDE 21      // LED VERDE conectado ao pino D2
#define DHT_TYPE DHT22   // Definição do tipo de sensor DHT
#define LDR 4
#define BOTAO_PIN 17
#define TRIG_PIN 5
#define ECHO_PIN 18
#define POTENCIOMETRO 27

 
// Instância do sensor DHT22 e Ultrasonic
DHT dht(DHT_PIN, DHT_TYPE);
Ultrasonic ultrasonic(TRIG_PIN, ECHO_PIN);

// Variaveis ADMIN

// const String SENHA = "ADMINGS"
const String hashSENHA = "6c0db579efe383b1ec2a86c95338a6e1b4c874378434f6b86d26d9f4808c5e05";
bool modoAdmin = false;
bool alertaEnviado = false;

// Variaveis Milis
unsigned long previousMillis = 0;
const long interval = 50; //1 segundo (para simular 30 minutos usar 1.800.000)
int Contador = 1;

 
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LDR, INPUT);
  pinMode(POTENCIOMETRO, INPUT);
  pinMode(BOTAO_PIN, INPUT_PULLUP);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
 
  Serial.println("Temperatura (ºC), Umidade (%), Luminosidade (LUX), Nivel da agua, Vibração do solo, Status Risco");
}

// Função para Gerar Hash com a entrada da senha
String gerarHashSHA256(String entrada) {
  SHA256 sha256;
  sha256.reset();
  sha256.update((const uint8_t*)entrada.c_str(), entrada.length());
  
  uint8_t resultado[32];
  sha256.finalize(resultado, sizeof(resultado));

  String hashHex = "";
  for (int i = 0; i < 32; i++) {
    if (resultado[i] < 16) hashHex += "0";
    hashHex += String(resultado[i], HEX);
  }

  return hashHex;
}

// Função para entrar no modo admin
void entrarModoAdmin() {
  const int tentativasMax = 3;
  int tentativas = tentativasMax;
  const unsigned long tempoBloqueio = 30000; // 30 segundos
  unsigned long bloqueioInicio = 0;

  Serial.println("Digite a senha ou 'EXIT' para sair:");

  while (true) {
    if (tentativas == 0) {
      bloqueioInicio = millis();
      Serial.println("Muitas tentativas incorretas. Aguardando 30 segundos...");

      // Espera sem travar o sistema pisca todos os leds
      while (millis() - bloqueioInicio < tempoBloqueio) {
        digitalWrite(LED_AMARELO, HIGH);
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(LED_VERMELHO, HIGH);
        delay(650);
        digitalWrite(LED_AMARELO, LOW);
        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_VERMELHO, LOW);
        delay(650);
      }

      tentativas = tentativasMax;
      Serial.println("Você pode tentar novamente. Digite a senha ou 'EXIT' para sair:");
    }

    if (Serial.available()) {
      String entrada = Serial.readStringUntil('\n');
      entrada.trim();

      if (entrada == "EXIT") {
        modoAdmin = false;
        Serial.println("Saindo do modo admin...");
        break;
      }

      String hashEntrada = gerarHashSHA256(entrada);
      if (hashEntrada == hashSENHA) {
        modoAdmin = true;
        Serial.println("Acesso concedido ao modo admin!");
        Serial.println("Data, ID, Temperatura (ºC), Umidade (%), Luminosidade (LUX), Nivel da agua, Vibração do solo, Status Risco, Saída ML");
        break;
      } else {
        tentativas--;
        Serial.print("Senha incorreta. Tentativas restantes: ");
        Serial.println(tentativas);
      }
    }
  }
}


void loop() {
  unsigned long currentMillis = millis();

  // inicio do Millis 
  if (currentMillis-previousMillis >= interval) {
    previousMillis = currentMillis;
    
    if(!modoAdmin){
      //Leitura dos sensores
      float temp = dht.readTemperature();
      float umid = dht.readHumidity();
      int dist = ultrasonic.distanceRead();
      int botao = digitalRead(BOTAO_PIN) == LOW ? 1:0;  
      int potenc = analogRead(POTENCIOMETRO); // Simular como se fosse um sensor SW-420 (digital)
      
      // Transformar ADC em LUX
      int ldr = analogRead(LDR);
      ldr = map(ldr, 4095, 0, 1024, 0);  // Converte a leitura do sensor LDR de um valor ADC do Arduino para um valor ADC do ESP32
      float tensao = ldr / 1024.0 * 5;  // Calcula a tensão com base na leitura do LDR
      float resistencia = 2000 * tensao / (1 - tensao / 5);  // Calcula a resistência do LDR
      float lux = pow(50 * 1e3 * pow(10, 0.7) / resistencia, (1 / 0.7));  // Calcula a luminosidade em lux

      // Simular como se fosse um sensor SW-420 (digital)
      bool vibracaoDetectada = (potenc > 3000); // valor ajustável
      bool nivelDetectado = (dist < 100); // valor ajustável
      bool luzDetectadaeNaoDetectada = (lux < 500 || lux > 2000); // True para lux < 500 (> 2000 lux, alertas)

      // Inicialização
      String riscosDetectados = "";
      bool alerta = false;
      bool perigo = false;

      // RISCO 1: Temperatura Extrema
      if (temp > 45.0) {
        riscosDetectados += "Calor extremo; ";
        perigo = true; // Temperatura perigosa
      } else if (temp > 35.0) {
        riscosDetectados += "Calor alto (alerta); ";
        alerta = true;
      } else if (temp < -5.0) {
        riscosDetectados += "Frio extremo; ";
        perigo = true;
      } else if (temp < 15.0) {
        riscosDetectados += "Frio baixo (alerta); ";
        alerta = true;
      }

      // RISCO 2: Umidade
      if (umid > 90.0) {
        riscosDetectados += "Umidade extrema; ";
        perigo = true;
      } else if (umid > 70.0) {
        riscosDetectados += "Umidade alta (alerta); ";
        alerta = true;
      } else if (umid < 10.0) {
        riscosDetectados += "Umidade muito baixa; ";
        perigo = true;
      } else if (umid < 30.0) {
        riscosDetectados += "Umidade baixa (alerta); ";
        alerta = true;
      }

      // RISCO 3: Luminosidade (fumaça ou chamas)
      if (lux < 100) {
        riscosDetectados += "Luminosidade extremamente baixa (fumaça densa/possível incêndio); ";
        perigo = true;
      } else if (lux >= 100 && lux < 500) {
        riscosDetectados += "Luminosidade baixa (fumaça moderada); ";
        alerta = true;
      } else if (lux >= 2000 && lux <= 8000) {
        riscosDetectados += "Luminosidade alta (luz intensa, alerta); ";
        alerta = true;
      } else if (lux > 8000) {
        riscosDetectados += "Luminosidade crítica (possível presença de chamas); ";
        perigo = true;
      }

      // RISCO 4: Nível da água (enchente)
      if (dist < 50) {
        riscosDetectados += "Enchente iminente; ";
        perigo = true;
      } else if (dist < 100) {
        riscosDetectados += "Nível da água elevado (alerta); ";
        alerta = true;
      }

      // RISCO 5: Vibração (deslizamento ou incêndio)
      if (potenc > 4000) {
        riscosDetectados += "Vibração forte detectada! Possível deslizamento; ";
        perigo = true;
      } else if (potenc > 3000) {
        riscosDetectados += "Vibração moderada (alerta); ";
        alerta = true;
      }

      // DEFININDO STATUS FINAL
      int ledStatus;
      String riscoResumo;

      if (perigo) {
        ledStatus = 2;  // LED VERMELHO
        riscoResumo = "PERIGO: " + riscosDetectados;
      } else if (alerta) {
        ledStatus = 1;  // LED AMARELO
        riscoResumo = "ALERTA: " + riscosDetectados;
      } else {
        ledStatus = 3;  // LED VERDE
        riscoResumo = "SEGURO: Nenhum risco detectado";
      }

      // Atualizando LEDs
      digitalWrite(LED_AMARELO,  ledStatus == 1 ? HIGH : LOW);
      digitalWrite(LED_VERMELHO, ledStatus == 2 ? HIGH : LOW);
      digitalWrite(LED_VERDE,    ledStatus == 3 ? HIGH : LOW);


      // Printar no formato CSV
      //Serial.printf(temp, (int)umid, ldr, nivel da agua, vibração do solo, riscoStatus);
      Serial.print(temp, 2);
      Serial.print(", ");
      Serial.print((int)umid);
      Serial.print(", ");
      Serial.print(luzDetectadaeNaoDetectada);
      Serial.print(", ");
      Serial.print(nivelDetectado);
      Serial.print(", ");
      Serial.print(vibracaoDetectada);
      Serial.print(", ");
      Serial.println(riscoResumo);
      Contador++;
    }
    // Se o botão for pressionado e não estiver no modo admin
    if (digitalRead(BOTAO_PIN) == LOW && !modoAdmin) {
      entrarModoAdmin();
    }

    // Modo Admin Ativo
    if (modoAdmin) {
      // Obter data e hora (simulada via millis)
      unsigned long totalMinutes = (Contador - 1) * 30;
      int hour = (totalMinutes / 60) % 24;
      int minute = totalMinutes % 60;
      int day = 1 + (totalMinutes / (24 * 60));

      //Leitura dos sensores
      float temp = dht.readTemperature();
      float umid = dht.readHumidity();
      int dist = ultrasonic.distanceRead();
      int botao = digitalRead(BOTAO_PIN) == LOW ? 1:0;  
      int potenc = analogRead(POTENCIOMETRO); // Simular como se fosse um sensor SW-420 (digital)
      
      // Transformar ADC em LUX
      int ldr = analogRead(LDR);
      ldr = map(ldr, 4095, 0, 1024, 0);  // Converte a leitura do sensor LDR de um valor ADC do Arduino para um valor ADC do ESP32
      float tensao = ldr / 1024.0 * 5;  // Calcula a tensão com base na leitura do LDR
      float resistencia = 2000 * tensao / (1 - tensao / 5);  // Calcula a resistência do LDR
      float lux = pow(50 * 1e3 * pow(10, 0.7) / resistencia, (1 / 0.7));  // Calcula a luminosidade em lux

      // Inicialização
      String riscosDetectados = "";
      bool alerta = false;
      bool perigo = false;

      // RISCO 1: Temperatura Extrema
      if (temp > 45.0) {
        riscosDetectados += "Calor extremo; ";
        perigo = true; // Temperatura perigosa
      } else if (temp > 35.0) {
        riscosDetectados += "Calor alto (alerta); ";
        alerta = true;
      } else if (temp < -5.0) {
        riscosDetectados += "Frio extremo; ";
        perigo = true;
      } else if (temp < 15.0) {
        riscosDetectados += "Frio baixo (alerta); ";
        alerta = true;
      }

      // RISCO 2: Umidade
      if (umid > 90.0) {
        riscosDetectados += "Umidade extrema; ";
        perigo = true;
      } else if (umid > 70.0) {
        riscosDetectados += "Umidade alta (alerta); ";
        alerta = true;
      } else if (umid < 10.0) {
        riscosDetectados += "Umidade muito baixa; ";
        perigo = true;
      } else if (umid < 30.0) {
        riscosDetectados += "Umidade baixa (alerta); ";
        alerta = true;
      }

      // RISCO 3: Luminosidade (fumaça ou chamas)
      if (lux < 100) {
        riscosDetectados += "Luminosidade extremamente baixa (fumaça densa/possível incêndio); ";
        perigo = true;
      } else if (lux >= 100 && lux < 500) {
        riscosDetectados += "Luminosidade baixa (fumaça moderada); ";
        alerta = true;
      } else if (lux >= 2000 && lux <= 8000) {
        riscosDetectados += "Luminosidade alta (luz intensa alerta); ";
        alerta = true;
      } else if (lux > 8000) {
        riscosDetectados += "Luminosidade crítica (possível presença de chamas); ";
        perigo = true;
      }

      // RISCO 4: Nível da água (enchente)
      if (dist < 50) {
        riscosDetectados += "Enchente iminente; ";
        perigo = true;
      } else if (dist < 100) {
        riscosDetectados += "Nível da água elevado (alerta); ";
        alerta = true;
      }

      // RISCO 5: Vibração (deslizamento ou incêndio)
      if (potenc > 4000) {
        riscosDetectados += "Vibração forte detectada! Possível deslizamento; ";
        perigo = true;
      } else if (potenc > 3000) {
        riscosDetectados += "Vibração moderada (alerta); ";
        alerta = true;
      }

      // DEFININDO STATUS FINAL
      int ledStatus;
      String riscoResumo;
      bool perigoDetectado = false;

      if (perigo) {
        ledStatus = 2;  // LED VERMELHO
        riscoResumo = "PERIGO: " + riscosDetectados;
        perigoDetectado = true; // <-- Rótulo de ML: PERIGO = 1
      } else if (alerta) {
        ledStatus = 1;  // LED AMARELO
        riscoResumo = "ALERTA: " + riscosDetectados;
      } else {
        ledStatus = 3;  // LED VERDE
        riscoResumo = "SEGURO: Nenhum risco detectado";
      }

      // Atualizando LEDs
      digitalWrite(LED_AMARELO,  ledStatus == 1 ? HIGH : LOW);
      digitalWrite(LED_VERMELHO, ledStatus == 2 ? HIGH : LOW);
      digitalWrite(LED_VERDE,    ledStatus == 3 ? HIGH : LOW);

        // Printar no formato CSV
      //Serial.printf("\nDia %d %02d:%02d, %d, %.2f, %d, %d, %d, %d", dia, hora, minuto, ID, temp, (int)umid, ldr, nivel da agua, vibração do solo, riscoStatus, target(ML));
      Serial.print("Dia ");
      Serial.print(day);
      Serial.print(" ");
      if (hour < 10) Serial.print("0");
      Serial.print(hour);
      Serial.print(":");
      if (minute < 10) Serial.print("0");
      Serial.print(minute);
      Serial.print(", ");
      Serial.print(Contador);
      Serial.print(", ");
      Serial.print(temp, 2);
      Serial.print(", ");
      Serial.print((int)umid);
      Serial.print(", ");
      Serial.print(lux);
      Serial.print(", ");
      Serial.print(dist);
      Serial.print(", ");
      Serial.print(potenc);
      Serial.print(", ");
      Serial.print(riscoResumo);
      Serial.print(", ");
      Serial.println(perigoDetectado ? "1" : "0");
      Contador++;
    }      
  }
}