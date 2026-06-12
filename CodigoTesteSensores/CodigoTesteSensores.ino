#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"

// ================= PINOS =================
#define PINO_NIVEL_AGUA 26
#define PINO_LDR 32

#define PINO_DHT 25
#define TIPO_DHT DHT11

#define PINO_RTC_SDA 14
#define PINO_RTC_SCL 27

// ================= OBJETOS =================
RTC_DS3231 rtc;
DHT dht(PINO_DHT, TIPO_DHT);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Iniciando sistema...");

  pinMode(PINO_NIVEL_AGUA, INPUT);
  pinMode(PINO_LDR, INPUT);

  dht.begin();

  Wire.begin(PINO_RTC_SDA, PINO_RTC_SCL);

  if (!rtc.begin()) {
    Serial.println("Erro: RTC não encontrado!");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC perdeu energia. Ajustando data e hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Sistema iniciado com sucesso!");
  Serial.println("--------------------------------");
}

void loop() {
  int leituraNivelAgua = analogRead(PINO_NIVEL_AGUA);
  int leituraLDR = analogRead(PINO_LDR);

  float umidadeAr = dht.readHumidity();
  float temperatura = dht.readTemperature();

  DateTime agora = rtc.now();

  Serial.print("Data/Hora: ");
  Serial.print(agora.day());
  Serial.print("/");
  Serial.print(agora.month());
  Serial.print("/");
  Serial.print(agora.year());
  Serial.print(" ");
  Serial.print(agora.hour());
  Serial.print(":");
  Serial.print(agora.minute());
  Serial.print(":");
  Serial.println(agora.second());

  Serial.print("Nivel de agua: ");
  Serial.println(leituraNivelAgua);

  Serial.print("LDR: ");
  Serial.println(leituraLDR);

  if (isnan(umidadeAr) || isnan(temperatura)) {
    Serial.println("Erro ao ler o DHT11");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Umidade do ar: ");
    Serial.print(umidadeAr);
    Serial.println(" %");
  }

  Serial.println("--------------------------------");

  delay(2000);
}