#include <RadioLib.h>
#include "heltec.h"
// Esp32 Heltec V3
// SX1262 has the following connections:
// NSS pin:   8
// DIO1 pin:  14
// NRST pin:  12
// BUSY pin:  13
SX1262 radio = new Module(8, 14, 12, 13);
#define BAND    915E6
//Variaveis globais
unsigned long startMillis = 0;
unsigned long currentMillis;

//Função para deixaros valores default SF=12;BW=500.0;PT=20
void SetDefaultParam(){
  
  //Inicializando os parametros default
  radio.setBandwidth(125.0);
  radio.setSpreadingFactor(12);
  radio.setOutputPower(20);
  //Serial.println("Parametros default setados");
  
}

//Função de troca de parâmetros
void changeParam(int SF, float BW, int PT){
  
  //Troca de parametros
  radio.setBandwidth(BW);
  radio.setSpreadingFactor(SF);
  radio.setOutputPower(PT);
  //Serial.println("TRC:SF"+String(SF)+"/BW"+String(BW)+"/PT"+String(PT));
}

//Função de inicialização de transmissão
void Send_HandShake(){
  
  //Inicializando as variaveis
  int state; //Variaveis de estado
  const unsigned long period = 5000;
  String msg_sync = "SYNC_HAND";
  String msg_sync_ack= "SYNC+ACK_HAND";
  String msg_ack = "ACK_HAND";
  String msg_read;
  bool loop = true;

  //Serial.println("Comecando HandShake");
  SetDefaultParam();
 
  while(loop){
    state = radio.transmit(msg_sync);
    radio.startReceive();
    //Serial.println("Enviou SYNC");
    
    while(currentMillis - startMillis <= period){
      currentMillis = millis();
      state = radio.receive(msg_read);
      //Serial.println("Recebendo msg");
      if(state == RADIOLIB_ERR_NONE && msg_read.equals(msg_sync_ack)){
         //Serial.print("Mensagem recebida: ");
         //Serial.println(msg_read);
         state = radio.transmit(msg_ack);
         radio.startReceive();
         //Serial.println("Enviou ACK");
         startMillis = currentMillis;
         loop = false;
         break;
      }
    }
    startMillis = currentMillis;
  }
  //Serial.println("Terminou HandShake");
}

String* Parser(String msg_read){
  static String valores[3];
    int cont = 0;
    int tam = msg_read.length();
    int aux = 0;
  //Parser da mensagem
    for(int i = 0; i<=tam; i++){
      if(msg_read.charAt(i) == '/'){
        valores[cont] = msg_read.substring(aux,i);
        aux=i+1;   
        cont++;
      }
    }
    return valores;
}

//Função de envio de pacotes
void SendPacket(){

  String msg_ack_packet = "ACK_PACKET";
  String packet = "Pacote ";
  int cont = 0;
  const unsigned long period = 15000; //alterar 
  String msg_read;
  int i;
 
  //Serial.println("Começando recebimento de pacote");
  for(i = 1; i <= 20; i++){
    Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(10, 5, "Enviando Pacote "+String(i));
  Heltec.display->display();
    String string_envio = packet+String(i);
    
    int state = radio.transmit(string_envio);
    Serial.print("Enviou o ");
    Serial.println(string_envio);
    delay(3000);
  }
  if(i == 21){
    delay(3000);
    String string_termino = "TERM";
    radio.transmit(string_termino);
    //Serial.println("Terminou recebimento de pacote");
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Heltec.begin(true /*Habilita o Display*/, false /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Habilita debug Serial*/, true /*Habilita o PABOOST*/, BAND /*Frequência BAND*/);
  
  // initialize SX1262 with default settings
  //Serial.print(F("Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    //Serial.println(F("success!"));
  } else {
    //Serial.print(F("failed, code "));
    //Serial.println(state);
    while (true);
  }
  if (radio.setFrequency(915.0) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    //Serial.println(F("Selected frequency is invalid for this module!"));
    while (true);
  }
  
  SetDefaultParam();
  Send_HandShake();
}

void loop() {
  SendPacket();
  
  SetDefaultParam();

  
  String msg_read;
  String msg_trc_ok = "TRC_OK";
  
  while(true){
    int state = radio.receive(msg_read);
    
    if(state == RADIOLIB_ERR_NONE && msg_read.equals(msg_trc_ok)){
    
      Serial.println("Encontrou os parametros Ideais");
      SendPacket();
      while(true){}
    }
    if(state == RADIOLIB_ERR_NONE && msg_read.charAt(0) == 'O' && msg_read.charAt(1) == 'K'){ //OK_12/125/10
      String *valores = Parser(msg_read.substring(2,msg_read.length()));
      changeParam(valores[0].toInt(),valores[1].toFloat(),valores[2].toInt());
      Serial.println("Retorna ao anterior");
      SendPacket();
      while(true){}
    }
    if(state == RADIOLIB_ERR_NONE){
      break;
    }
  }
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(30, 5, "Trocando de parâmetro");
  Heltec.display->display();
  String *valores = Parser(msg_read);
   changeParam(valores[0].toInt(),valores[1].toFloat(),valores[2].toInt());
   Serial.println(String(valores[0].toInt())+";"+String(valores[1].toFloat())+";"+String(valores[2].toInt()));

}
