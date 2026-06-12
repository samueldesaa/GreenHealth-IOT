#include <WiFi.h>
#include <PubSubClient.h>

// ================= WIFI =================
const char* ssid = "S20FE";
const char* password = "samuelsa";

// ================= MQTT =================
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic = "samuel/esp32/cores";

// ================= ENCODER =================
const int ENCODER_CLK = 26;
const int ENCODER_DT  = 25;
const int ENCODER_SW  = 33;

// ================= LEDS DE PREVIEW =================
const int LED_R = 32;

// GPIO35 não funciona como saída.
// Troquei para GPIO27.
const int LED_G = 27;

// ================= CONFIG =================
const int PASSO = 5;

WiFiClient espClient;
PubSubClient client(espClient);

volatile int encoderDelta = 0;
volatile int lastCLK = HIGH;

int valorR = 0;
int valorG = 0;

unsigned long ultimoBotao = 0;
const unsigned long debounceBotao = 250;

enum Modo {
  AJUSTANDO_R,
  AJUSTANDO_G,
  AGUARDANDO_GIRO
};

Modo modoAtual = AJUSTANDO_R;

void IRAM_ATTR lerEncoder() {
  int clk = digitalRead(ENCODER_CLK);
  int dt = digitalRead(ENCODER_DT);

  if (clk != lastCLK) {
    if (clk == LOW) {
      if (dt == HIGH) {
        encoderDelta++;
      } else {
        encoderDelta--;
      }
    }
    lastCLK = clk;
  }
}

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

void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT... ");

    String clientId = "ESP32_Encoder_";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente...");
      delay(2000);
    }
  }
}

void atualizarPreview() {
  analogWrite(LED_R, valorR);
  analogWrite(LED_G, valorG);
}

void publicarValores() {
  String mensagem = String(valorR) + "," + String(valorG);

  client.publish(mqtt_topic, mensagem.c_str());

  Serial.print("Publicado: ");
  Serial.println(mensagem);
}

void mostrarEstado() {
  Serial.print("R: ");
  Serial.print(valorR);
  Serial.print(" | G: ");
  Serial.print(valorG);
  Serial.print(" | Modo: ");

  if (modoAtual == AJUSTANDO_R) {
    Serial.println("ajustando vermelho");
  } else if (modoAtual == AJUSTANDO_G) {
    Serial.println("ajustando verde");
  } else {
    Serial.println("aguardando novo giro");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);

  lastCLK = digitalRead(ENCODER_CLK);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), lerEncoder, CHANGE);

  conectarWiFi();

  client.setServer(mqtt_server, mqtt_port);

  Serial.println("Sistema iniciado");
  Serial.println("Gire para ajustar R");
  Serial.println("Clique para ajustar G");
  Serial.println("Clique novamente para publicar");
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }

  client.loop();

  int movimento = 0;

  noInterrupts();
  movimento = encoderDelta;
  encoderDelta = 0;
  interrupts();

  if (movimento != 0) {
    if (modoAtual == AGUARDANDO_GIRO) {
      modoAtual = AJUSTANDO_R;
    }

    if (modoAtual == AJUSTANDO_R) {
      valorR += movimento * PASSO;
      valorR = constrain(valorR, 0, 255);
    } else if (modoAtual == AJUSTANDO_G) {
      valorG += movimento * PASSO;
      valorG = constrain(valorG, 0, 255);
    }

    atualizarPreview();
    mostrarEstado();
  }

  if (digitalRead(ENCODER_SW) == LOW && millis() - ultimoBotao > debounceBotao) {
    ultimoBotao = millis();

    if (modoAtual == AJUSTANDO_R) {
      modoAtual = AJUSTANDO_G;
      Serial.println("Agora ajustando o LED verde");
    } else if (modoAtual == AJUSTANDO_G) {
      publicarValores();
      modoAtual = AGUARDANDO_GIRO;
      Serial.println("Valores enviados. LEDs continuam ligados até novo giro.");
    }

    mostrarEstado();
  }
}