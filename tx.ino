#include <Arduino.h>

#define TX_PIN 17
#define RX_PIN 16

uint8_t sequencia = 0;
const uint8_t MARCADOR = 0x7E;

uint8_t calcularChecksum(uint8_t* dados, uint8_t tamanho) {
  uint16_t soma = 0;
  for (uint8_t i = 0; i < tamanho; i++) soma += dados[i];
  while (soma > 0xFF) soma = (soma & 0xFF) + (soma >> 8);
  return (uint8_t)(~soma & 0xFF);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("TX Iniciado");
}

void loop() {
  uint8_t payload[2] = {0x1A, 0x2B};
  uint8_t comp = 3;

  uint8_t dadosParaProteger[4] = {sequencia, comp, payload[0], payload[1]};
  
  unsigned long inicio = micros();
  uint8_t checksum = calcularChecksum(dadosParaProteger, 4);
  unsigned long tempoCalculo = micros() - inicio;
  
  Serial.print("Tempo calculo checksum: ");
  Serial.print(tempoCalculo);
  Serial.println("us");

  Serial2.write(MARCADOR);
  Serial2.write(sequencia);
  Serial2.write(comp);
  Serial2.write(payload[0]);
  Serial2.write(payload[1]);
  Serial2.write(checksum);
  Serial2.write(payload[0]);
  Serial2.write(payload[1]);

  sequencia++;
  delay(50);
}
