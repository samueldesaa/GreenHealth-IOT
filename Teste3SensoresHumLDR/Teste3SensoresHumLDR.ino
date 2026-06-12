#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "DHT.h"
#include "RTClib.h"

// ================= WIFI =================
const char* WIFI_SSID = "S20FE";
const char* WIFI_PASSWORD = "samuelsa";

// ================= MQTT =================
const char* MQTT_BROKER = "broker.emqx.io";
const int MQTT_PORT = 1883;

const char* MQTT_CLIENT_ID = "esp32_greenhealth_samuel_001";

// ================= API DE HORÁRIO =================
const char* TIME_API_URL = "https://timeapi.io/api/TimeZone/zone?timeZone=America/Fortaleza";

// ================= TÓPICOS GERAIS =================
const char* TOPICO_TEMPERATURA = "greenhealth/sensores/temperatura";
const char* TOPICO_UMIDADE_AR = "greenhealth/sensores/umidade_ar";
const char* TOPICO_DATA_HORA = "greenhealth/sensores/data_hora";
const char* TOPICO_JSON = "greenhealth/sensores/dados";

// ================= OBJETOS WIFI/MQTT =================
WiFiClient espClient;
PubSubClient mqtt(espClient);

// ================= DHT =================
#define DHTPIN 27
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ================= RTC =================
#define RTC_SDA 26
#define RTC_SCL 25
#define RTC_ADDRESS 0x68

RTC_DS3231 rtc;
bool rtcDetectado = false;

// ================= CONFIGURAÇÃO DAS PLANTAS =================
#define MAX_PLANTAS 10

struct Planta {
  int numero;
  int pinoLdr;
  int pinoUmidade;
  bool ativa;
};

Planta plantas[MAX_PLANTAS];
int totalPlantas = 0;

// ================= CONTROLE DE ENVIO =================
unsigned long ultimoEnvio = 0;
const unsigned long intervaloEnvio = 1000;

unsigned long ultimoStatusSerial = 0;
const unsigned long intervaloStatusSerial = 5000;

// ================= ADICIONAR PLANTA =================
void adicionarPlanta(int numero, int pinoLdr, int pinoUmidade) {
  if (totalPlantas >= MAX_PLANTAS) {
    Serial.println("Limite máximo de plantas atingido!");
    return;
  }

  if (pinoLdr == DHTPIN || pinoUmidade == DHTPIN) {
    Serial.println("ERRO: Uma planta está tentando usar o mesmo pino do DHT!");
    Serial.print("Pino do DHT: GPIO ");
    Serial.println(DHTPIN);
    Serial.println("Altere o pino do LDR ou da umidade dessa planta.");
    Serial.println();
    return;
  }

  plantas[totalPlantas].numero = numero;
  plantas[totalPlantas].pinoLdr = pinoLdr;
  plantas[totalPlantas].pinoUmidade = pinoUmidade;
  plantas[totalPlantas].ativa = true;

  pinMode(pinoLdr, INPUT);
  pinMode(pinoUmidade, INPUT);

  Serial.print("Planta ");
  Serial.print(numero);
  Serial.println(" configurada.");

  Serial.print("LDR: GPIO ");
  Serial.println(pinoLdr);

  Serial.print("Umidade do solo: GPIO ");
  Serial.println(pinoUmidade);

  Serial.println();

  totalPlantas++;
}

// ================= FUNÇÃO WIFI =================
void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi conectado!");

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ================= FUNÇÃO MQTT =================
void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao MQTT... ");

    if (mqtt.connect(MQTT_CLIENT_ID)) {
      Serial.println("conectado!");
    } else {
      Serial.print("falhou. Estado: ");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// ================= IMPRIMIR STATUS NO SERIAL =================
void imprimirStatusConexao() {
  Serial.println("========== STATUS DE CONEXÃO ==========");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi: conectado");

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("WiFi: desconectado");
  }

  if (mqtt.connected()) {
    Serial.println("MQTT: conectado");
  } else {
    Serial.println("MQTT: desconectado");

    Serial.print("Estado MQTT: ");
    Serial.println(mqtt.state());
  }

  Serial.print("RTC: ");
  Serial.println(rtcDetectado ? "detectado" : "não detectado");

  Serial.print("Plantas ativas: ");
  Serial.println(totalPlantas);

  Serial.print("Tempo ligado: ");
  Serial.print(millis() / 1000);
  Serial.println(" segundos");

  Serial.println("========================================");
  Serial.println();
}

// ================= PUBLICAR VALOR INTEIRO =================
void publicarInt(const char* topico, int valor) {
  char mensagem[20];
  sprintf(mensagem, "%d", valor);
  mqtt.publish(topico, mensagem);
}

// ================= PUBLICAR VALOR FLOAT =================
void publicarFloat(const char* topico, float valor) {
  char mensagem[20];
  dtostrf(valor, 6, 2, mensagem);
  mqtt.publish(topico, mensagem);
}

// ================= VERIFICAR SE RTC ESTÁ PRESENTE =================
bool verificarRtcI2C() {
  Wire.beginTransmission(RTC_ADDRESS);
  byte erro = Wire.endTransmission();

  return erro == 0;
}

// ================= FORMATAR DATA E HORA DO RTC =================
String formatarDataHoraRTC(DateTime agora) {
  char dataHora[25];

  sprintf(
    dataHora,
    "%02d/%02d/%04d %02d:%02d:%02d",
    agora.day(),
    agora.month(),
    agora.year(),
    agora.hour(),
    agora.minute(),
    agora.second()
  );

  return String(dataHora);
}

// ================= CONFIGURAR RTC =================
void configurarRTC() {
  Wire.begin(RTC_SDA, RTC_SCL);
  delay(100);

  if (rtc.begin() && verificarRtcI2C()) {
    rtcDetectado = true;
    Serial.println("RTC DS3231 identificado.");

    if (rtc.lostPower()) {
      Serial.println("RTC perdeu energia. Ajustando com data/hora da compilação.");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    rtcDetectado = false;
    Serial.println("RTC não identificado. O sistema usará a API de horário.");
  }
}

// ================= EXTRAIR CAMPO SIMPLES DO JSON DA API =================
String extrairCampoJson(String json, String campo) {
  String chave = "\"" + campo + "\":\"";
  int inicio = json.indexOf(chave);

  if (inicio == -1) {
    return "";
  }

  inicio += chave.length();

  int fim = json.indexOf("\"", inicio);

  if (fim == -1) {
    return "";
  }

  return json.substring(inicio, fim);
}

// ================= FORMATAR DATA DA API =================
String formatarDataHoraApi(String dataHoraApi) {
  if (dataHoraApi.length() == 0) {
    return "horario_indisponivel";
  }

  dataHoraApi.replace("T", " ");

  if (dataHoraApi.length() >= 19) {
    dataHoraApi = dataHoraApi.substring(0, 19);
  }

  if (dataHoraApi.length() >= 19) {
    String ano = dataHoraApi.substring(0, 4);
    String mes = dataHoraApi.substring(5, 7);
    String dia = dataHoraApi.substring(8, 10);
    String hora = dataHoraApi.substring(11, 19);

    return dia + "/" + mes + "/" + ano + " " + hora;
  }

  return dataHoraApi;
}

// ================= OBTER DATA E HORA POR API =================
String obterDataHoraAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    return "wifi_desconectado";
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(5000);

  Serial.println("Buscando horário pela API...");

  if (!http.begin(client, TIME_API_URL)) {
    Serial.println("Erro ao iniciar conexão com API de horário.");
    return "erro_api";
  }

  int codigoHttp = http.GET();

  if (codigoHttp != 200) {
    Serial.print("Erro na API de horário. Código HTTP: ");
    Serial.println(codigoHttp);

    http.end();
    return "erro_api";
  }

  String resposta = http.getString();
  http.end();

  String dataHora = extrairCampoJson(resposta, "currentLocalTime");

  if (dataHora == "") {
    dataHora = extrairCampoJson(resposta, "dateTime");
  }

  if (dataHora == "") {
    dataHora = extrairCampoJson(resposta, "datetime");
  }

  dataHora = formatarDataHoraApi(dataHora);

  Serial.print("Horário obtido pela API: ");
  Serial.println(dataHora);

  return dataHora;
}

// ================= OBTER DATA E HORA PRINCIPAL =================
// Primeiro tenta RTC.
// Se o RTC não for identificado ou parar de responder, usa API.
String obterDataHora() {
  if (rtcDetectado && verificarRtcI2C()) {
    DateTime agora = rtc.now();
    String dataHoraRTC = formatarDataHoraRTC(agora);

    Serial.print("Horário obtido pelo RTC: ");
    Serial.println(dataHoraRTC);

    return dataHoraRTC;
  }

  if (rtcDetectado) {
    Serial.println("RTC parou de responder. Alternando para API.");
  }

  rtcDetectado = false;

  // Tenta detectar novamente o RTC, caso ele tenha voltado
  if (rtc.begin() && verificarRtcI2C()) {
    rtcDetectado = true;
    Serial.println("RTC detectado novamente. Voltando a usar RTC.");

    DateTime agora = rtc.now();
    String dataHoraRTC = formatarDataHoraRTC(agora);

    Serial.print("Horário obtido pelo RTC: ");
    Serial.println(dataHoraRTC);

    return dataHoraRTC;
  }

  return obterDataHoraAPI();
}

// ================= PUBLICAR DADOS DAS PLANTAS =================
String montarJsonPlantas() {
  String jsonPlantas = "\"plantas\":[";

  bool primeiraPlantaNoJson = true;

  for (int i = 0; i < totalPlantas; i++) {
    if (!plantas[i].ativa) {
      continue;
    }

    int valorLdr = analogRead(plantas[i].pinoLdr);
    int valorUmidade = analogRead(plantas[i].pinoUmidade);

    int numero = plantas[i].numero;

    char topicoLdr[60];
    char topicoUmidade[60];

    sprintf(topicoLdr, "greenhealth/planta%d/ldr", numero);
    sprintf(topicoUmidade, "greenhealth/planta%d/umidade", numero);

    publicarInt(topicoLdr, valorLdr);
    publicarInt(topicoUmidade, valorUmidade);

    if (!primeiraPlantaNoJson) {
      jsonPlantas += ",";
    }

    jsonPlantas += "{";
    jsonPlantas += "\"planta\":" + String(numero) + ",";
    jsonPlantas += "\"ldr\":" + String(valorLdr) + ",";
    jsonPlantas += "\"umidade_solo\":" + String(valorUmidade);
    jsonPlantas += "}";

    primeiraPlantaNoJson = false;
  }

  jsonPlantas += "]";

  return jsonPlantas;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // ================= CONFIGURE AS PLANTAS AQUI =================
  // Formato:
  // adicionarPlanta(numero, pinoLdr, pinoUmidade);

  adicionarPlanta(1, 33, 32);
  adicionarPlanta(2, 33, 35);
  adicionarPlanta(3, 33, 34);

  // ================= DHT =================
  dht.begin();

  // ================= RTC =================
  configurarRTC();

  // ================= WIFI =================
  conectarWiFi();

  // ================= MQTT =================
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  conectarMQTT();
}

// ================= LOOP =================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  if (!mqtt.connected()) {
    conectarMQTT();
  }

  mqtt.loop();

  unsigned long agoraMillis = millis();

  // ================= IMPRIME STATUS NO TERMINAL =================
  if (agoraMillis - ultimoStatusSerial >= intervaloStatusSerial) {
    ultimoStatusSerial = agoraMillis;
    imprimirStatusConexao();
  }

  // ================= ENVIO DOS SENSORES =================
  if (agoraMillis - ultimoEnvio >= intervaloEnvio) {
    ultimoEnvio = agoraMillis;

    float temperatura = dht.readTemperature();
    float umidadeAr = dht.readHumidity();

    // Primeiro usa RTC. Se falhar, usa API.
    String dataHora = obterDataHora();

    // ================= PUBLICA DHT =================
    if (!isnan(temperatura)) {
      publicarFloat(TOPICO_TEMPERATURA, temperatura);
    }

    if (!isnan(umidadeAr)) {
      publicarFloat(TOPICO_UMIDADE_AR, umidadeAr);
    }

    // ================= PUBLICA DATA E HORA =================
    mqtt.publish(TOPICO_DATA_HORA, dataHora.c_str());

    // ================= MONTA JSON GERAL =================
    String json = "{";

    json += montarJsonPlantas();
    json += ",";

    if (!isnan(temperatura)) {
      json += "\"temperatura\":" + String(temperatura, 2) + ",";
    } else {
      json += "\"temperatura\":null,";
    }

    if (!isnan(umidadeAr)) {
      json += "\"umidade_ar\":" + String(umidadeAr, 2) + ",";
    } else {
      json += "\"umidade_ar\":null,";
    }

    json += "\"data_hora\":\"" + dataHora + "\"";

    json += "}";

    mqtt.publish(TOPICO_JSON, json.c_str());

    Serial.println("Dados enviados via MQTT:");
    Serial.println(json);
    Serial.println();
  }
}