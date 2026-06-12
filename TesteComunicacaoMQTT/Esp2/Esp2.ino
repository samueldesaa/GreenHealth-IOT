#include <WiFi.h>
#include <PubSubClient.h>

// ================= WIFI =================
const char* ssid = "S20FE";
const char* password = "samuelsa";

// ================= MQTT =================
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic = "samuel/esp32/cores";

// ================= LEDS =================
const int LED_R = 13;
const int LED_G = 12;

WiFiClient espClient;
PubSubClient client(espClient);

int valorR = 0;
int valorG = 0;

void conectarWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Mensagem recebida: ");
  Serial.println(mensagem);

  int separador = mensagem.indexOf(',');

  if (separador > 0) {
    valorR = mensagem.substring(0, separador).toInt();
    valorG = mensagem.substring(separador + 1).toInt();

    valorR = constrain(valorR, 0, 255);
    valorG = constrain(valorG, 0, 255);

    analogWrite(LED_R, valorR);
    analogWrite(LED_G, valorG);

    Serial.print("LED R: ");
    Serial.print(valorR);
    Serial.print(" | LED G: ");
    Serial.println(valorG);
  }
}

void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT... ");

    String clientId = "ESP32_Receptor_";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");

      client.subscribe(mqtt_topic);

      Serial.print("Inscrito no tópico: ");
      Serial.println(mqtt_topic);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);

  conectarWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }

  client.loop();
}