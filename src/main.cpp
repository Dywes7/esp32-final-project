#include <Arduino.h>
#include <dht.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>

/* """"""Sensor de Temperatura Secion"""""" */
//DEFINIÇÃO DE PINOS
#define pinSensor 4

//INTERVALO DE LEITURA
#define intervalo 5000

//CRIANDO VARIAVEIS E INSTANCIANDO OBJETOS
unsigned long delayIntervalo;
dht sensorDHT;


/* """"""Recetor Infravermelho Secion"""""" */
const uint16_t kRecvPin = 14;
IRrecv irrecv(kRecvPin);
decode_results results;


/* """"""Emissor Infravermelho Secion"""""" */
const uint16_t kIrLed = 5;
IRsend irsend(kIrLed);


// Declaração das funções das tarefas
void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);

void setup() {
  // Inicialização do monitor serial
  Serial.begin(115200);

  irsend.begin();

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
    1000,    // Tamanho da pilha
    NULL,    // Parâmetro para a tarefa
    1,       // Prioridade da tarefa
    NULL);   // Handle da tarefa

  xTaskCreate(
    Task2,   // Função da tarefa
    "Task 2", // Nome da tarefa
    10000,    // Tamanho da pilha
    NULL,    // Parâmetro para a tarefa
    1,       // Prioridade da tarefa
    NULL);   // Handle da tarefa

  xTaskCreate(
    Task3,   // Função da tarefa
    "Task 3", // Nome da tarefa
    10000,    // Tamanho da pilha
    NULL,    // Parâmetro para a tarefa
    1,       // Prioridade da tarefa
    NULL);   // Handle da tarefa
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
  

      // EXIBINDO DADOS LIDOS
      Serial.print(stop - start);
      Serial.print(", \t\t");
      Serial.print(sensorDHT.humidity, 1 /*FORMATAÇÃO PARA UMA CASA DECIMAL*/);
      Serial.print(",\t\t");
      Serial.println(sensorDHT.temperature, 1 /*FORMATAÇÃO PARA UMA CASA DECIMAL*/);

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
void Task3(void *pvParameters) {

  while (1) {

    Serial.println("Botao apertado!");
    irsend.sendNEC(0x57E3E817);
    delay(2000);

  }
  
}
