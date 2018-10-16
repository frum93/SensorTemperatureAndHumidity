#include <dht.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>

//INICIALIZAÇÕES
LiquidCrystal_I2C lcd(0x3F, 16, 2);

//Rede
const char* ssid     = "Janela";        // Nome da Rede
const char* password = "abretesesamo";      // Password da rede
const char* host = "api.thingspeak.com";
const char* THINGSPEAK_API_KEY = "YC4D3GIEXRCX49WS";    //Write API KEY

//Atualização à taxa de 1 minuto, o Thingspeak necessita no minimo 20segundo
const int UPDATE_INTERVAL_SECONDS = 60;

int botao = 13;                                       // botão no pino 7
int estado_botao = 0;                                 // Variável que conterá os estados do botão (0 LOW, 1 HIGH)
unsigned long tempo_anterior_lcd = 0;                 // cria uma Variável que guarde o tempo anterior essa variável não terá números negativos(LCD)
const long intervalo_lcd = 6000;                      // Indica o tempo que o ecra vai ficar com luminosidade ligada(LCD)
unsigned long tempo_anterior_leitura = 0;             // cria uma Variável que guarde o tempo anterior essa variável não terá números negativos
const long intervalo_tempo_leitura = 5000;            // indica o intervalo de tempo em milissegundos pretendido para o sensor fazer a leitura e mostrar no ecrã
unsigned long tempo_anterior_thingspeak = 0;          // cria uma Variável que guarde o tempo anterior essa variável não terá números negativos(ThingSpeak)
const long intervalo_tempo_thingspeak = 1200000;       // 10minutos. É o intervalo de tempo que os dados são enviados para a plaaforma ThingSpeak
bool estado = false ;
long espera = 10000;
long tempo = 0;
// Configurações sensor DHT22

#define DHTPIN 2                         // Utilização D6 do NodeMCU
#define DHTTYPE DHT22

// Inicialização do sensor de temperature e humidade
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);      // Inicia o "lcd" de 2 linhas e 16 colunas
  lcd.init();
  pinMode(botao, INPUT);    //Define o botão como entrada
  lcd.backlight();
}

void loop() {

  if (estado == false && millis() - tempo >= espera) {
    tempo = millis();
    WiFi.begin(ssid, password); // Inicia a ligação a rede
    if (WiFi.status() == WL_CONNECTED) {
      estado = true;
    } else {
      estado = false;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    estado = false;
  }


  float h = dht.readHumidity();         //Leitura da Humidade
  float t = dht.readTemperature();      //Leitura da Temperatura

  if (millis() - tempo_anterior_thingspeak >= intervalo_tempo_thingspeak) {
    tempo_anterior_thingspeak = millis();

    Serial.print("connecting to ");
    Serial.println(host);//Apresenta no monitor série o nome da rede à qual é efectuada a ligação

    // Uso WiFiClient para criar comunicações TCP
    WiFiClient client;
    const int httpPort = 80; //Uso Porto80
    if (!client.connect(host, httpPort)) {
      Serial.println("Falha Comunicação");//Verifica o estado da ligação
      return;
    }

    // Criar URL para o pedido
    String url = "/update?api_key=";
    url += THINGSPEAK_API_KEY;
    url += "&field1="; //Colocar os dados de temperatura no gráfico 1 do Thingspeak
    url += String(t);
    url += "&field2="; //Colocar os dados de humidade no gráfico 2 do Thingspeak
    url += String(h);
    Serial.print("Pedido URL: ");
    Serial.println(url);

    // Envio de solicitação ao servidor
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    while (!client.available()) { //Verificação se o cliente está conectado
      Serial.print(".");
    }

    // Leitura das respostas do servidor e envio para o monitor série
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("Comunicação fechada"); //Depois do cliente efectuar o pedido apresenta esta mensagem no monitor série
  }


  estado_botao = digitalRead(botao);
  if (estado_botao == HIGH) {
    tempo_anterior_lcd = millis();
    lcd.backlight();
  }

  if (millis() - tempo_anterior_lcd >= intervalo_lcd) {
    lcd.noBacklight();
    tempo_anterior_lcd = 0;
  }

  if (millis() - tempo_anterior_leitura >= intervalo_tempo_leitura) {
    tempo_anterior_leitura = millis();
    lcd.setCursor(0, 0);
    lcd.print("Temperatura:");
    lcd.print(t);
    lcd.setCursor(0, 1);
    lcd.print("Humidade:");
    lcd.print(h);
  }
}


