//Importação das bibliotecas necessárias
#include "WiFi.h"
#include "HTTPClient.h"
#include "DHTesp.h"
#include "ThingSpeak.h"
#include "PubSubClient.h"

//SSID = Nome da rede, password = senha da rede
const char* ssid = "Wokwi-GUEST";
const char* password = "";

//Pino da esp32 a qual o sensor enviará os dados
const int DHT_PIN = 13;

//CHANNEL, WRITE_API, server, são respectivamentes o canal, chave de API e o servidor do thingspeak
//Ambos os valores são adquiriros ao criar um canal 
unsigned long CHANNEL = 2143264; 
const char* WRITE_API = "3JI5T9JVS7CXTE5P";
const char* server = "api.thingspeak.com";

//Pinos a qual estão ligadas as alimentações das leds vermelha e amarela
int led1 = 18;
int led2 = 19;

//Valores referentes a temperatura e umidade minimas para fornecer alimentação às leds citdas acima
int medTemperatura = 35; //35°C
int medUmidade = 70; //70%

//Delay utilizado para controlar o tempo das funções
unsigned long timerDelay = 15000; //15000ms == 15s

DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {

  //Inicializando o sensor, WiFi e o ThingSpeak
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22); //Pino a qual o sensor esta conectado juntamente ao modelo do sensor
  pinMode(led1, OUTPUT); //Pino a qual o led vermelho esta ligado juntamente ao tipo de saída
  pinMode(led2, OUTPUT); //Pino a qual o led amarelo esta ligado juntamento ao tipo de saída
  delay(10);

  //Tentativas de se conectar a internet
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Conected!.");
  Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_STA);

  //Iniciando a conectividade entre o dispostivo e a plataforma ThingSpeak
  ThingSpeak.begin(espClient);  
  Serial.println("");
  Serial.println("Conectado!");

  //Tentativas de se conectar ao broker
  Serial.println("Conectando ao Broker Mqtt");
  mqttClient.setServer("broker.hivemq.com", 1883);
  String clientID = "esp32_Grupo1-" + String(random(0xffff), HEX);  
  boolean isConected = mqttClient.connect(clientID.c_str());
  while(!isConected){
    delay(500);
    String clientID = "esp32_Grupo1-" + String(random(0xffff), HEX);
    isConected = mqttClient.connect(clientID.c_str());
    Serial.print(".");
  }
  Serial.println("Conectado!");
}

void loop() {
  //Funções que retornam o valor recebido do sensor
  float temperatura = dhtSensor.getTemperature();
  float umidade = dhtSensor.getHumidity();
  Serial.println("Temperatura: " + String(temperatura) + "°C");
  Serial.println("Umidade: " + String(umidade) + "%");

  //Enviar uma mensagem em formato json para o tópico "senai" acompanhado da "mensagem"
  char mensagem[100];
  sprintf(mensagem, "{\"temperatura\" : %.2f, \"umidade\" : %.2f}", temperatura, umidade);
  mqttClient.publish("senai", mensagem);

  //Condicional que fornece ou corta alimentação dos pinos
  if(temperatura > medTemperatura){
    digitalWrite(led1, HIGH);
  }
  else {
    digitalWrite(led1, LOW);
  }

  if(umidade > medUmidade){
    digitalWrite(led2, HIGH);
  }
  else {
    digitalWrite(led2, LOW);
  }

  //Prepara para a plataforma ThingSpeak nos campos 1 e 2 os valores das variáveis temperatura e umidade
  ThingSpeak.setField(1, temperatura);
  ThingSpeak.setField(2, umidade);

  //Escreve no canal do ThingSepeak
  int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
  if(x == 200){
    Serial.println("Atualização feita com sucesso");
    } 
  else 
    {
    Serial.println("Erro HTTP " + String(x));
  }
  delay (timerDelay);
}
