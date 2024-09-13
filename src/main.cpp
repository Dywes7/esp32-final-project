#include <Arduino.h>
#include <dht.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ir_Samsung.h>


/* """"""Telegra* Section """""" */
#define BOTtoken "6965994152:AAFRiiVZGw56Vt0yliTM6chPSwcdMXz5JIE"
#define CHAT_ID "5567601893"

const char* ssid = "GRT-TI";
const char* password = "&*(suporte)_+";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
bool buttonPressed = false;


/* """"""Sensor de Temperatura Secion"""""" */
//DEFINIÇÃO DE PINOS
#define pinSensor 4

//INTERVALO DE LEITURA
#define intervalo 2000

//CRIANDO VARIAVEIS E INSTANCIANDO OBJETOS
unsigned long delayIntervalo;
dht sensorDHT;


/* """"""Receptor Infravermelho Secion"""""" */
const uint16_t kRecvPin = 14;
IRrecv irrecv(kRecvPin);
decode_results results;  


/* """"""LedEmissor Infravermelho Secion"""""" */
// const uint16_t kIrLed = 5;
// IRsend irsend(kIrLed);

const uint16_t kIrLed = 5;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRSamsungAc ac(kIrLed);     // Set the GPIO used for sending messages.


String irCommand = ""; // Variável global para armazenar o comando IR

float globalTemperature = 0.0;
float globalHumidity = 0.0;
String globalStatus = "OK"; // Status do sensor

String stateAtual = "";

/* Adicionando variável global para controle de tempo */
unsigned long lastCommandTime = 0; // Armazena o tempo do último comando enviado
const unsigned long commandInterval = 20 * 1000; // Intervalo de 5 minutos (em milissegundos)

String fakeCommand = "";  // Variável global para comando fictício
int lastSetTemp = 0;

// Declaração das funções das tarefas
void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);
void Task4(void *pvParameters);
void handleNewMessages(int numNewMessages);
String getStateString();
void printState();

void printState() {
  // Display the settings.
  Serial.println("Samsung A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
}

void setup() {

  ac.begin();
  Serial.begin(115200);
  delay(200);

  // Set up what we want to send. See ir_Samsung.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting initial state for A/C.");
  ac.on();
  ac.setFan(kSamsungAcFanLow);
  ac.setMode(kSamsungAcCool);
  ac.setTemp(25);
  ac.setSwing(false);
  stateAtual = getStateString();
  printState();

  irrecv.enableIRIn();
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
  
  //IMPRIMINDO INFORMAÇÕES SOBRE A BIBLIOTECA
  Serial.print("VERSAO DA BIBLIOTECA: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  // Serial.println("Status,\tTempo(uS),\tUmidade(%),\tTemperatura(C)");

  // Criação das tarefas
  xTaskCreate(
    Task1,   // Função da tarefa
    "Task 1", // Nome da tarefa
    5000,    // Tamanho da pilha
    NULL,    // Parâmetro para a tarefa
    1,       // Prioridade da tarefa
    NULL);   // Handle da tarefa

  xTaskCreate(
    Task2,   // Função da tarefa
    "Task 2", // Nome da tarefa
    10000,    // Tamanho da pilha
    NULL,    // Parâmetro para a tarefa
    2,       // Prioridade da tarefa
    NULL);   // Handle da tarefa
 
  //xTaskCreate(
  //  Task3,   // Função da tarefa
  //  "Task 3", // Nome da tarefa
  //  10000,    // Tamanho da pilha
  //  NULL,    // Parâmetro para a tarefa
  //  4,       // Prioridade da tarefa
  //  NULL);   // Handle da tarefa

    xTaskCreate(
    Task4,   // Função da tarefa
    "Task 4", // Nome da tarefa
    10000,    // Tamanho da pilha
    NULL,    // Parâmetro para a tarefa
    3,       // Prioridade da tarefa
    NULL);   // Handle da tarefa


    // Inicializar conexão Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate f
    #endif

    while (WiFi.status() != WL_CONNECTED) {

      delay(1000);
      Serial.println("Connecting to WiFi..");

    }
    // Printar enderec¸o IP local
    Serial.println(WiFi.localIP());
    
}

void loop() {
  // O loop pode ficar vazio, pois as tarefas estão sendo gerenciadas pelo FreeRTOS
}

// Medidor de Temperatura
void Task1(void *pvParameters) {
  while (1) {
    // Serial.println("Task 1 is running");

    if ( (millis() - delayIntervalo) > intervalo ) {
      //LEITURA DOS DADOS
      unsigned long start = micros();
      int chk = sensorDHT.read22(pinSensor);
      unsigned long stop = micros();

      Serial.println("\nStatus,\tTempo(uS),\tUmidade(%),\tTemperatura(C)");
  
      // VERIFICA SE HOUVE ERRO
      switch (chk)
      {
      case DHTLIB_OK:
          Serial.print("OK,\t");
          break;
      case DHTLIB_ERROR_CHECKSUM:
          Serial.print("Checksum error,\t");
          break;
      case DHTLIB_ERROR_TIMEOUT:
          Serial.print("Time out error,\t");
          break;
      case DHTLIB_ERROR_CONNECT:
          Serial.print("Connect error,\t");
          break;
      case DHTLIB_ERROR_ACK_L:
          Serial.print("Ack Low error,\t");
          break;
      case DHTLIB_ERROR_ACK_H:
          Serial.print("Ack High error,\t");
          break;
      default:
          Serial.print("Unknown error,\t");
          break;
      }


      if (chk == DHTLIB_OK) {

        globalTemperature = sensorDHT.temperature;

      }

      globalHumidity = sensorDHT.humidity;

      // EXIBINDO DADOS LIDOS
      Serial.print(stop - start);
      Serial.print(", \t\t");
      Serial.print(sensorDHT.humidity, 1 /*FORMATAÇÃO PARA UMA CASA DECIMAL*/);
      Serial.print(",\t\t");
      Serial.println(sensorDHT.temperature, 1 /*FORMATAÇÃO PARA UMA CASA DECIMAL*/);


      // Obter a temperatura atual configurada no ar-condicionado
      int currentTemp = ac.getTemp();
      

      if ((millis() - lastCommandTime) >= commandInterval) {
        // Verificar se a temperatura ambiente é maior ou igual a 25°C
        if (globalTemperature >= 24.0) {
            Serial.println("Temperatura real: " + String(globalTemperature));
          // Verificar se a temperatura configurada no ar-condicionado é maior que 16°C
          if (currentTemp > 16) {
            // Diminuir a temperatura em 1 grau
            ac.setTemp(currentTemp - 1);
            ac.send(); // Enviar comando infravermelho

            Serial.print("Temperatura no ar-condicionado ajustada para: ");
            Serial.println(currentTemp - 1);

            // Enviar mensagem via Telegram
            lastSetTemp = currentTemp - 1;  // Atualizar a temperatura ajustada
            fakeCommand = "diminuir_temp";  // Disparar comando fictício

            Serial.println("Chegou ate aqui 1!!!");

            // Atualizar o tempo do último comando
            lastCommandTime = millis();
          }
        }
        // Verificar se a temperatura ambiente é menor ou igual a 20°C
        else if (globalTemperature <= 22.0) {
          Serial.println("Temperatura real: " + String(globalTemperature));
          // Verificar se a temperatura configurada no ar-condicionado é menor que 30°C
          if (currentTemp < 30) {
            // Aumentar a temperatura em 1 grau
            ac.setTemp(currentTemp + 1);
            ac.send(); // Enviar comando infravermelho

            Serial.print("Temperatura no ar-condicionado ajustada para: ");
            Serial.println(currentTemp + 1);

            // Enviar mensagem via Telegram
            lastSetTemp = currentTemp + 1;  // Atualizar a temperatura ajustada
            fakeCommand = "aumentar_temp";  // Disparar comando fictício

            Serial.println("Chegou ate aqui 2!!!");

            // Atualizar o tempo do último comando
            lastCommandTime = millis();
          }
        } else {
          fakeCommand = "";
          Serial.println("Temperatura Ideal.");
        }
      } else {
        // Caso o intervalo de 5 minutos não tenha passado, exibir mensagem no Serial Monitor
        Serial.println("Aguardando 20 segundos antes de enviar um novo comando...");
      }


      delayIntervalo = millis();
    };

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay de 1 segundo
  }
}

// Receptor Infravermelho
void Task2(void *pvParameters) {
  while (1) {
    // Serial.println("Task 2 is running");

    if (irrecv.decode(&results)) {
        // print() & println() can't handle printing long longs. (uint64_t)
        serialPrintUint64(results.value, HEX);
        Serial.println("");
        irrecv.resume();  // Receive the next value
      }
      delay(100);

//    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay de 2 segundos
  }
}

// Emissor Infravermelho
//void Task3(void *pvParameters) {
//
//  while (1) {
//
//    // Turn the A/C unit on
//  Serial.println("Turn on the A/C ...");
//  ac.on();
//  ac.send();
//  stateAtual = getStateString();
//  printState();
//  delay(10000);  // wait 15 seconds
//
//  Serial.println("Turn off the A/C ...");
//  ac.off();
//  ac.send();
//  stateAtual = getStateString();
//  printState();
//  delay(10000);  // wait 15 seconds
//
//  }
//  
//}


// TELEGRAM ESPERANDO POR NOVA MENSAGEM
void Task4(void *pvParameters) {

  while (1) {

    if (fakeCommand != "") {
      Serial.println("Comando fictício detectado.");
      handleNewMessages(0);  // Chama handleNewMessages com 0 mensagens novas, mas ainda processa o comando fictício
    }

    if (millis() > lastTimeBotRan + botRequestDelay) {

      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      if (numNewMessages > 0) {

        handleNewMessages(numNewMessages);

      }

      lastTimeBotRan = millis();

    }

    delay(1000); // Ajuste o atraso conforme necessário

  }
  
}

void handleNewMessages(int numNewMessages) {

    Serial.println("handleNewMessages");
    Serial.println(String(numNewMessages));

    if (fakeCommand != "") {
    String chat_id = CHAT_ID;
    String message;

    if (fakeCommand == "diminuir_temp") {
      message = "A temperatura do ar-condicionado foi **diminuída** para " + String(lastSetTemp) + "°C.";
    } else if (fakeCommand == "aumentar_temp") {
      message = "A temperatura do ar-condicionado foi **aumentada** para " + String(lastSetTemp) + "°C.";
    }

    // Enviar a mensagem correspondente ao comando fictício
    bot.sendMessage(chat_id, message, "");

    // Reseta o comando fictício para não processá-lo novamente
    fakeCommand = "";
    return;  // Não processar mais nada se o comando fictício foi enviado
    }

    for (int i=0; i<numNewMessages; i++) {
        // Chat id of the requester
        String chat_id = String(bot.messages[i].chat_id);

        if (chat_id != CHAT_ID){

            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;

        }

        // Print the received message
        String text = bot.messages[i].text;
        Serial.println(text);
        String from_name = bot.messages[i].from_name;

        if (text == "/start") {

            String welcome = "Welcome, " + from_name + ".\n";
            welcome += "Use the following commands to control your outputs.\n\n";
            welcome += "/teste para to try the infravermelho \n";
            bot.sendMessage(chat_id, welcome, "");

        } else if (text == "/temperatura") {

            // Use the global variables to construct the message
            String message = "Status, Tempo(uS), Umidade(%), Temperatura(C)\n";
            message += globalStatus;
            message += ", ";
            message += String(delayIntervalo); // Tempo não é diretamente disponível, você pode ajustar conforme necessário
            message += ", ";
            message += String(globalHumidity, 1); // Uma casa decimal
            message += ", ";
            message += String(globalTemperature, 1); // Uma casa decimal
            bot.sendMessage(chat_id, message, "");


        } else if (text =="/state") {
          
          bot.sendMessage(chat_id, stateAtual, "");

        } else {
          
            irCommand = text;
            char message[256];
            snprintf(message, sizeof(message), "Comando realizado: %s", text);
            bot.sendMessage(chat_id, message, "");

           // bot.sendMessage(chat_id, "Comando não reconhecido :(", "");

        }

    }

}


String getStateString() {

  String stateString = "Samsung A/C remote is in the following state:\n";
  stateString += ac.toString().c_str();
  return stateString;

}