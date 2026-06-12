# 🌱 GreenHealth IoT

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![ESP32](https://img.shields.io/badge/hardware-ESP32-blue)
![MQTT](https://img.shields.io/badge/protocolo-MQTT-green)
![IoT](https://img.shields.io/badge/projeto-IoT-brightgreen)
![Node-RED](https://img.shields.io/badge/Node--RED-futuro-red)
![Arduino](https://img.shields.io/badge/Arduino_IDE-00979D?logo=arduino&logoColor=white)
![HTML5](https://img.shields.io/badge/HTML5-E34F26?logo=html5&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?logo=javascript&logoColor=black)

---

## 📌 Nome do projeto

**GreenHealth IoT**

O **GreenHealth IoT** é uma versão reduzida do projeto GreenHealth, desenvolvida com foco em **Internet das Coisas** para o monitoramento de plantas domésticas.

Nesta versão, o sistema utiliza um **ESP32** conectado a sensores para coletar dados do ambiente e das plantas, enviando essas informações por meio do protocolo **MQTT** para uma interface web.

---

## 🌿 Problema

O cuidado com plantas domésticas geralmente é feito de forma manual e intuitiva. Isso pode dificultar o acompanhamento correto das condições da planta e do ambiente, causando problemas como:

- Falta de água;
- Excesso de água;
- Baixa luminosidade;
- Dificuldade de acompanhar temperatura e umidade do ar;
- Ausência de dados para apoiar decisões sobre o cuidado da planta.

Dessa forma, o projeto busca responder ao seguinte problema:

> Como utilizar IoT para monitorar as condições ambientais de plantas domésticas e auxiliar o usuário no acompanhamento da umidade, luminosidade, temperatura e umidade do ar?

---

## 🎯 Objetivo do projeto

Desenvolver o GREENHEALTH, um sistema inteligente para monitoramento e irrigação automatizada de plantas domésticas, capaz de utilizar dados ambientais, informações climáticas e perfis botânicos para apoiar o cuidado individualizado, ampliar a autonomia da irrigação e permitir o acompanhamento remoto pelo usuário. 


O sistema permite acompanhar:

- 💧 Umidade do solo;
- ☀️ Luminosidade;
- 🌡️ Temperatura do ambiente;
- 🌫️ Umidade do ar;
- 📡 Status de conexão MQTT;
- 🕒 Data e hora das leituras.

---

## 🧠 Funcionamento atual

Atualmente, o projeto está estruturado para realizar a leitura dos sensores no ESP32 e enviar os dados coletados para um broker MQTT.

A interface web recebe essas informações e apresenta os dados ao usuário por meio de cards, gráficos e indicadores visuais.

```txt
🌱 Sensores
   ↓
📟 ESP32
   ↓ MQTT
☁️ Broker EMQX
   ↓ WebSocket
💻 Dashboard Web
   ↓
👤 Usuário
```

Nesta etapa, o foco principal está no **monitoramento ambiental** das plantas, permitindo visualizar os dados em tempo real.

---

## 🧰 Componentes usados

### 🔌 Hardware

- ESP32;
- Sensores de umidade do solo;
- Sensores LDR para luminosidade;
- Sensor DHT para temperatura e umidade do ar;
- RTC DS3231;
- Jumpers;
- Protoboard;
- Cabo USB ou fonte de alimentação.

### 💻 Software e tecnologias

- Arduino IDE;
- HTML;
- CSS;
- JavaScript;
- MQTT;
- Broker EMQX;
- PubSubClient;
- WiFi;

---

## 🔌 Esquema de ligação

| Componente | Pino no ESP32 | Observação |
|---|---|---|
| Sensor de umidade do solo 1 | GPIO 26 | Entrada analógica |
| Sensor de umidade do solo 2 | GPIO 33 | Entrada analógica |
| Sensor de umidade do solo 3 | GPIO 35 | Entrada analógica |
| LDR 1 | GPIO 25 | Entrada analógica |
| LDR 2 | GPIO 32 | Entrada analógica |
| LDR 3 | GPIO 34 | Entrada analógica |
| DHT | GPIO 27 | Entrada digital |
| RTC SDA | GPIO 12 | Comunicação I2C |
| RTC SCL | GPIO 13 | Comunicação I2C |
| VCC dos sensores | 3V3 ou 5V | Conforme o módulo utilizado |
| GND dos sensores | GND | Terra comum |

> ⚠️ **Atenção:** os pinos GPIO 34, 35, 36 e 39 do ESP32 são apenas entrada. Eles podem ser usados para sensores, mas não devem ser usados como saída.

---

## 📡 Comunicação MQTT

O projeto utiliza o protocolo **MQTT** para comunicação entre o ESP32 e a interface web.

| Item | Valor |
|---|---|
| Broker utilizado | `broker.emqx.io` |
| Porta MQTT no ESP32 | `1883` |
| Porta WebSocket para o dashboard | `8084` |

### Exemplo de tópicos MQTT

```txt
greenhealth/temperatura
greenhealth/umidade_ar
greenhealth/data_hora
greenhealth/dados
```

Também é possível enviar todos os dados em um único tópico no formato JSON:

```json
{
  "temperatura": 28.5,
  "umidade_ar": 70,
  "solo1": 45,
  "solo2": 52,
  "solo3": 38,
  "luz1": 1800,
  "luz2": 2100,
  "luz3": 1650,
  "data_hora": "11/06/2026 18:30"
}
```

---

## 🖥️ Dashboard web

A interface web foi desenvolvida para exibir os dados recebidos do ESP32 em tempo real.

### Recursos atuais da interface

- Cards com valores atuais dos sensores;
- Gráficos de linha em tempo real;
- Status de conexão com o MQTT;
- Visualização dos sensores de umidade;
- Visualização dos sensores de luminosidade;
- Visualização da temperatura e umidade do ar;
- Atualização automática dos dados.

---

## 🚀 Como usar

### 1. Clonar o repositório

```bash
git clone https://github.com/seu-usuario/greenhealth-iot.git
```

### 2. Abrir o código do ESP32

Abra o arquivo na Arduino IDE:

```txt
codigo/greenhealth_iot.ino
```

### 3. Instalar as bibliotecas necessárias

Na Arduino IDE, instale as seguintes bibliotecas:

- WiFi;
- PubSubClient;
- DHT sensor library;
- RTClib.

### 4. Configurar Wi-Fi

No código do ESP32, altere os dados da rede Wi-Fi:

```cpp
const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DA_REDE";
```

### 5. Configurar o broker MQTT

Verifique se o broker está configurado corretamente:

```cpp
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
```

### 6. Conectar os sensores

Monte o circuito seguindo a tabela de conexões apresentada neste README.

### 7. Enviar o código para o ESP32

Na Arduino IDE:

1. Selecione a placa ESP32 correta;
2. Escolha a porta COM correspondente;
3. Clique em **Upload**;
4. Abra o **Monitor Serial** para verificar a conexão Wi-Fi e MQTT.

### 8. Abrir o dashboard

Abra o arquivo:

```txt
web/index.html
```

Ou publique a pasta `web/` em uma plataforma como:

- GitHub Pages;
- Vercel;
- Netlify.

---

## 📌 Estado atual do projeto

| Módulo | Status |
|---|---|
| Repositório criado | ✅ Concluído |
| Código atual do ESP32 | ✅ Implementado |
| Leitura dos sensores | ✅ Implementado |
| Envio de dados por MQTT | ✅ Implementado |
| Dashboard web | ✅ Implementado |
| Exibição de gráficos | ✅ Implementado |
| Status de conexão MQTT | ✅ Implementado |
| Data e hora das leituras | ✅ Implementado |
| README com descrição do projeto | ✅ Implementado |
| Esquema de ligação | ✅ Parcial |
| Irrigação automatizada | 🔜 A implementar |
| Reservatórios locais com servos | 🔜 A implementar |
| Node-RED para tratamento dos dados MQTT | 🔜 A implementar |

---

## 🔮 O que falta implementar

Para as próximas etapas do projeto, espera-se implementar:

- 💧 **Irrigação automatizada**, permitindo que o sistema controle a liberação de água para as plantas;
- 🧴 **Reservatórios locais**, em que cada planta poderá ter seu próprio armazenamento de água;
- ⚙️ **Liberação de água por servos**, utilizando microservos para controlar o fluxo de irrigação;
- 🔴 **Node-RED**, para criação de fluxos de tratamento das informações recebidas via MQTT;
- 📊 Processamento dos dados antes da exibição no dashboard;
- 🚨 Possíveis alertas para baixa umidade do solo, excesso de água ou luminosidade inadequada;
- 🗃️ Histórico de leituras para acompanhamento da evolução das condições das plantas.

---

## 🧪 Versão reduzida para IoT

Esta versão representa uma adaptação simplificada do projeto GreenHealth original.

Ela mantém o foco no **monitoramento ambiental** e na **comunicação IoT**, mas ainda não inclui todos os recursos planejados para a versão completa, como automação da irrigação, perfis botânicos, recomendações inteligentes e integração avançada com serviços externos.

---

## 📚 Aprendizados

Durante o desenvolvimento do projeto foram trabalhados conceitos como:

- Leitura de sensores no ESP32;
- Comunicação Wi-Fi;
- Comunicação MQTT;
- Integração entre hardware e web;
- Uso de JSON;
- Criação de dashboard;
- Gráficos em tempo real;
- Organização de projeto no GitHub;
- Planejamento de automação com sensores e atuadores.

---

## 📦 Entrega do projeto

Este repositório contém:

- ✅ Código atual do projeto;
- ✅ README com nome do projeto;
- ✅ Descrição do problema;
- ✅ Lista de componentes usados;
- ✅ Explicação do funcionamento;
- ✅ Instruções básicas de uso;
- ✅ Indicação do que ainda falta implementar.

---

## 👨‍💻 Autor

Desenvolvido por **Samuel Chaves de Sá**.

Projeto acadêmico desenvolvido no curso de **Sistemas de Informação**, com foco em **IoT, automação, monitoramento ambiental e cuidado inteligente de plantas domésticas**.

---

## 📄 Licença

Este projeto é de uso acadêmico e experimental.

Sinta-se livre para estudar, adaptar e evoluir a ideia. 🌱