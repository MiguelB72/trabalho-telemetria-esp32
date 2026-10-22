#include <Arduino.h>

#define TX_PIN 17
#define RX_PIN 16

const uint8_t MARCADOR = 0x7E;
uint8_t buffer[7];
uint8_t qtd = 0;
bool recebendo = false;
unsigned long ultimoByte = 0;
const unsigned long TIMEOUT = 60; // 60ms (margem sobre 50ms do TX)

uint8_t seqEsperada = 0;
bool primeiraLeitura = true;

uint8_t calcularChecksum(uint8_t* dados, uint8_t tamanho) {
  uint16_t soma = 0;
  for (uint8_t i = 0; i < tamanho; i++) soma += dados[i];
  while (soma > 0xFF) soma = (soma & 0xFF) + (soma >> 8);
  return (uint8_t)(~soma & 0xFF);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Seq,Residuo,Decisao,PayloadConfere,TempoVerificacao");
}

void loop() {
  while (Serial2.available() > 0) {
    uint8_t b = Serial2.read();
    ultimoByte = millis();

    if (!recebendo) {
      if (b == MARCADOR) { recebendo = true; qtd = 0; }
    } else {
      if (b == MARCADOR && qtd > 0) { qtd = 0; continue; }
      buffer[qtd++] = b;
      if (qtd == 7) {
        uint8_t seq = buffer[0];
        uint8_t comp = buffer[1];
        uint8_t p1 = buffer[2];
        uint8_t p2 = buffer[3];
        uint8_t chk = buffer[4];
        uint8_t ref1 = buffer[5];
        uint8_t ref2 = buffer[6];

        unsigned long inicio = micros();
        uint8_t dadosParaChecar[5] = {seq, comp, p1, p2, chk};
        uint8_t residuo = calcularChecksum(dadosParaChecar, 5);
        unsigned long tempo = micros() - inicio;

        bool chkOk = (residuo == 0x00);
        bool compOk = (comp == 3);
        bool payloadOk = (p1 == ref1 && p2 == ref2);
        const char* decisao = (chkOk && compOk) ? "Aceito" : "Descartado";

        Serial.print(seq); Serial.print(",");
        Serial.print(residuo, HEX); Serial.print(",");
        Serial.print(decisao); Serial.print(",");
        Serial.print(payloadOk ? "Sim" : "Nao"); Serial.print(",");
        Serial.println(tempo);

        // Lacuna na sequência
        if (!primeiraLeitura && seq != seqEsperada) {
          Serial.print("# AVISO: salto de sequencia; esperada=");
          Serial.print(seqEsperada);
          Serial.print(", recebida=");
          Serial.println(seq);
        }
        seqEsperada = seq + 1;
        primeiraLeitura = false;

        recebendo = false;
        qtd = 0;
      }
    }
  }
  if (recebendo && millis() - ultimoByte > TIMEOUT) {
    Serial.println("# Timeout");
    recebendo = false;
    qtd = 0;
  }
}
