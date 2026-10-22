#include <Arduino.h>
#include "esp_timer.h" 

#define PINO_SNIFFER 16
#define PINO_INJETOR 4

int modoRuido = 0;
uint32_t contadorQuadros = 0;

esp_timer_handle_t timerPulso = NULL;
volatile unsigned long duracaoPulsoAtual = 0;

void IRAM_ATTR aoDispararTimer(void* arg) {
  digitalWrite(PINO_INJETOR, HIGH);
  delayMicroseconds(duracaoPulsoAtual); 
  digitalWrite(PINO_INJETOR, LOW);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, PINO_SNIFFER, -1);
  pinMode(PINO_INJETOR, OUTPUT);
  digitalWrite(PINO_INJETOR, LOW); 
  
  const esp_timer_create_args_t timer_args = {
      .callback = &aoDispararTimer,
      .arg = NULL,
      .name = "timer_ruido"
  };
  esp_timer_create(&timer_args, &timerPulso);
  
  Serial.println("# Injetor de ruido com Hardware Timer iniciado.");
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c >= '0' && c <= '3') {
      modoRuido = c - '0';
      Serial.print("# Modo alterado para: ");
      Serial.println(modoRuido);
    }
  }

  while (Serial2.available() > 0) {
    uint8_t byteLido = Serial2.read();

    if (byteLido == 0x7E) {
      contadorQuadros++;
      
      if (modoRuido > 0) {
        if (modoRuido == 1) duracaoPulsoAtual = 30;
        else if (modoRuido == 2) duracaoPulsoAtual = 50;
        else duracaoPulsoAtual = 80;
        
        unsigned long timestamp = micros();
        int posicaoEstimada = 180 / 87; // ~2 bytes após 0x7E

        Serial.print("Quadro: "); Serial.print(contadorQuadros);
        Serial.print(" | Modo: "); Serial.print(modoRuido);
        Serial.print(" | Posicao estimada: byte ~"); Serial.print(posicaoEstimada);
        Serial.print(" | Duracao: "); Serial.print(duracaoPulsoAtual);
        Serial.print("us | Timestamp: "); Serial.print(timestamp);
        Serial.println("us | Pulso aplicado");

        esp_timer_start_once(timerPulso, 180); 
      }
    }
  }
}
