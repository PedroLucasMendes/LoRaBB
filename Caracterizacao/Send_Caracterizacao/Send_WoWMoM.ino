#include <RadioLib.h>

// Esp32 Heltec V3
// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
SX1262 radio = new Module(8, 14, 12, 13);

//Variaveis globais
unsigned long startMillis = 0;
unsigned long currentMillis;


//Vetores Spreading Factor, BandWidth and Transmission Power
int Vetor_SF[] = {7,8,9,10,11,12}; //6
float Vetor_BW[] = {125,250,500}; //3
int Vetor_PT[] = {10,12,14,16,18,20}; //6

void SetDefaultParam(){
  
  //Inicializando os parametros default
  radio.setBandwidth(125.0);
  radio.setSpreadingFactor(12);
  radio.setOutputPower(20);
  //Serial.println("Parametros default setados");
  
}

void changeParam(int SF, float BW, int PT){
  
  //Troca de parametros
  radio.setBandwidth(BW);
  radio.setSpreadingFactor(SF);
  radio.setOutputPower(PT);
  Serial.println("TRC:SF"+String(SF)+"/BW"+String(BW)+"/PT"+String(PT));
}


void Send_HandShake(){
  
  //Inicializando as variaveis
  int state; //Variaveis de estado
  const unsigned long period = 5000;
  String msg_sync = "SYNC_HAND";
  String msg_sync_ack= "SYNC+ACK_HAND";
  String msg_ack = "ACK_HAND";
  String msg_read;
  bool loop = true;

  Serial.println("Comecando HandShake");
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
  Serial.println("Terminou HandShake");
  
}

void SendPacket(){

  String msg_ack_packet = "ACK_PACKET";
  String packet = "Pacote ";
  int cont = 0;
  const unsigned long period = 15000; //alterar 
  String msg_read;
  int i;

  Serial.println("Começando recebimento de pacote");
  for(i = 1; i <= 100; i++){
    String string_envio = packet+String(i);
    int state = radio.transmit(string_envio);
    radio.startReceive();
    Serial.println(string_envio);
    startMillis = currentMillis;
    while(currentMillis - startMillis <= period){
       currentMillis = millis();
       state = radio.receive(msg_read);
       if(state == RADIOLIB_ERR_NONE && msg_read.equals(msg_ack_packet)){
           Serial.print("MSGR");
           //Serial.print(msg_read);
           Serial.print(";"+String(radio.getRSSI()));
           Serial.println(";"+String(radio.getSNR()));
           break;
       }
    }
    //Verificador se perdeu pacote
    if(currentMillis - startMillis >= period){
      Serial.print("PDR");
      Serial.print(";"+String(radio.getRSSI()));
      Serial.println(";"+String(radio.getSNR()));
      cont++;
    }
    startMillis = currentMillis;
    //Verificação se perdeu 5 pacotes
    if(cont >= 50){
      Serial.println("PDR 50");
      i = 102;
    }
  }
  if(i == 101){
    String string_termino = "TERM";
    radio.transmit(string_termino);
    Serial.println("Terminou recebimento de pacote");
  }
}


void setup() {
  Serial.begin(115200);
  while(!Serial);
  
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

  Send_HandShake();

}

void loop() {

  //Variaveis de inicialização
  const unsigned long period = 5000;

  
  //Mensagens
  String msg_troca_ack = "TROCA_ACK";
  String msg_read;

  Serial.println("Começando recebimento da mensagem de troca");
  for(int SF = 0; SF < 6; SF++){
    for(int BW = 0; BW < 3; BW++){
      for(int PT = 0; PT < 11; PT++){
        bool loop = true;
        SetDefaultParam();
        //7/125.0/12
        String string_troca = String(Vetor_SF[SF])+"/"+String(Vetor_BW[BW])+"/"+String(Vetor_PT[PT])+"/";
        
        while(loop){
          int state = radio.transmit(string_troca);
          //Serial.println("Enviou "+string_troca);
          
          startMillis = currentMillis;
          while(currentMillis - startMillis <= period){
            currentMillis = millis();
            state = radio.receive(msg_read);
            //Serial.println("Recebendo msg");
            if(state == RADIOLIB_ERR_NONE && msg_read.equals(msg_troca_ack)){
               //Serial.print("Mensagem recebida: ");
               //Serial.println(msg_read);
               
               changeParam(Vetor_SF[SF], Vetor_BW[BW], Vetor_PT[PT]);
               SendPacket();
               
               startMillis = currentMillis;
               loop = false;
               break;
            }
          }
          startMillis = currentMillis;
        }
        Serial.println("Trocando de parametro");
      }
    }
  }

  Serial.println("Experimentos terminou");
  while(true){}
}
