#include <RadioLib.h>
#include "heltec.h"
#include <algorithm>
#define BAND    915E6

//ESP32 Heltec V2
// SX1276 has the following connections:
// NSS pin:   18
// DIO1 pin:  26
// NRST pin:  14
// BUSY pin:  35 
SX1276 radio = new Module(18, 26, 14, 35);
unsigned long currentMillis;
unsigned long startMillis = 0;

int Vetor_SF[] = {7,8,9,10,11,12}; //6
float Vetor_BW[] = {125,250,500}; //3
int Vetor_PT[] = {10,12,14,16,18,20}; //6

int valor_SF = 12;
float valor_BW = 125.00;
int valor_PT = 20;

String msg_ult;

int posicao_vetor = 10;
int vetor_param[11] = {10,12,14,16,18,7,8,9,10,11,12};

//Função para deixaros valores default SF=12;BW=125.0;PT=20
void SetDefaultParam(){
  
  //Inicializando os parametros default
  radio.setBandwidth(125.0);
  radio.setSpreadingFactor(12);
  radio.setOutputPower(20);
  //Serial.println("Parametros default setados");
  
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

//Função de troca de parâmetros
void changeParam(int SF, float BW, int PT){
  
  //Troca de parametros
  radio.setBandwidth(BW);
  radio.setSpreadingFactor(SF);
  radio.setOutputPower(PT);
  //Serial.println("TRC:SF"+String(SF)+"/BW"+String(BW)+"/PT"+String(PT));
}

//Função de inicialização de transmissão
void Receiver_HandShake(){
  /*
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  
  Heltec.display->drawString(30, 5, "HandShake");
  Heltec.display->display();
  */
  
  unsigned long period = 5000; // Tempo de espera em milissegundos
  int state = 0;
  String msg_sync = "SYNC_HAND";
  String str_read;
  bool loop = true;

  SetDefaultParam();
  
  while(true){
    state = radio.receive(str_read);
    if(state == RADIOLIB_ERR_NONE && str_read.equals(msg_sync)){
      break;
    }
  }
  
  while(loop){
    
    state = radio.transmit("SYNC+ACK_HAND");
    while(currentMillis - startMillis <= period) {
      currentMillis = millis();  
      String ack_hand = "ACK_HAND";
      state = radio.receive(str_read);
      if(state == RADIOLIB_ERR_NONE && str_read.equals(ack_hand)){
          loop = false;
          break;
      }
    }
  }
}

//Função de recebimento de pacote
void Receiver_SendPacket(int SF, float BW,  int PT){

  unsigned long period = 15000*100; // 1 minutos
  String str_read;
  String str_finish = "TERM";
  bool loop = true;

  //Serial.println("Começando recebimento de pacote");
  /*
    Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  
  Heltec.display->drawString(30, 5, "Send Packet");
  Heltec.display->display();
  */
  currentMillis = millis();
  startMillis = currentMillis;
  while(loop){
    currentMillis = millis();
    //Serial.println("Esperando mensagens");

    int state = radio.receive(str_read);
    if(state == RADIOLIB_ERR_NONE && str_read.equals(str_finish)){
      //Serial.print("Mensagem recebida: ");
      //Serial.println(str_read);
      startMillis = currentMillis;  
      loop = false;
    }
    else if(state == RADIOLIB_ERR_NONE){
      
      //Serial.print("Mensagem recebida: ");
      //Serial.println(str_read);
      
      //Serial.print(String(SF)+";"+String(BW)+";"+String(PT));
      //Serial.print(";"+String(radio.getRSSI()));
      //Serial.println(";"+String(radio.getSNR()));
      startMillis = currentMillis;
      /*
        Heltec.display->clear();
      Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
      Heltec.display->setFont(ArialMT_Plain_16);
      
      Heltec.display->drawString(30, 5, "Packet Receiver");
      Heltec.display->display();  
      */
    }
    if(currentMillis - startMillis >= period){
       startMillis = currentMillis;   
       //Serial.println("SEMPACTRC");
       loop = false;
    }
     // Heltec.display->clear();    
  }
  //Serial.println("Terminou recebimento de pacote");
}

int getSF(){
  return valor_SF;
}

float getBW(){
  return valor_BW;
}

int getPT(){
  return valor_PT;
}

//Implementação dos algoritmos
int verificaParam(float vetor_RSSI[],float vetor_SNR[],int contador_perda){
  
  float maiorSNR = 0;
  int tamanho = sizeof(vetor_RSSI) / sizeof(vetor_RSSI[0]);
  for(int i = 0; i < tamanho;i++){
    if(maiorSNR < vetor_SNR[i]){
      maiorSNR = vetor_SNR[i]+20;  
    }
  }
  
  float dr = 0;
  int margin = 15;
  if(valor_SF == 12){
    dr = -20;
  }else if(valor_SF == 11){
    dr = -17.5;
  }else if(valor_SF == 10){
    dr = -15;
  }else if(valor_SF == 9){
    dr = -12.5;
  }else if(valor_SF == 8){
    dr = -10;
  }else if(valor_SF == 7){
    dr = -7.5;
  }

  float SNRmargin = maiorSNR - dr - margin;

  int Nstep = SNRmargin/2.5;
  //Serial.println("Nstep: "+String(Nstep));
  
  //Algoritmo LoRaWAN
  
  if(Nstep == 0){
    return 0;
  }
  while(Nstep != 0){
     if(Nstep > 0){
      posicao_vetor--;
      if(posicao_vetor < 0){
        //Serial.println("SF: "+String(valor_SF));
        //Serial.println("PT: "+String(valor_PT));
        //Serial.println("return 01");
        return 0;
      }
      if(valor_SF == 7){
        valor_PT = vetor_param[posicao_vetor];
      }else{
        valor_SF = vetor_param[posicao_vetor];
      }
      Nstep--;
      //Serial.println("Step--");
    }else{
      if(valor_PT < 20){
          valor_PT += 2;
          Nstep++;
      }else{
        //Serial.println("SF: "+String(valor_SF));
        //Serial.println("PT: "+String(valor_PT));
        //Serial.println("return 02");
        return 0;
      }
    }
    //Serial.println("STEP: "+String(Nstep));
    //Serial.println("posVetor: "+String(posicao_vetor));
  }
  //Serial.println("SF: "+String(valor_SF));
  //Serial.println("PT: "+String(valor_PT));
  //Serial.println("return 03");
  return 1;
}

void setup() {

  Serial.begin(115200);
  while(!Serial);
  //Heltec.begin(true /*Habilita o Display*/, false /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Habilita debug Serial*/, true /*Habilita o PABOOST*/, BAND /*Frequência BAND*/);
  
  /*
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->clear();
  Heltec.display->drawString(33, 5, "Iniciado");
  Heltec.display->drawString(10, 30, "com Sucesso!");
  Heltec.display->display();
  */
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
  } else {
    while (true);
  }
  if (radio.setFrequency(915.0) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    while (true);
  }
  SetDefaultParam();
  Receiver_HandShake();
}

void loop() {

  String str_read;
  int contador = 0;
  unsigned long period = 1000*20;
  
  float vetor_RSSI[20];
  float vetor_SNR[20];

  int v_Verifica;
  /*
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(30, 5, "Recebimento TRC");
  Heltec.display->display();
  */
  Serial.println("COM_VER");
  float startMillis = millis();
  float currentMillis = 0;
    
    while(true){
      currentMillis = millis();
      int state = radio.receive(str_read);
  
      if(currentMillis-startMillis >= period){
        break;
      }
      if(state == RADIOLIB_ERR_NONE && str_read.equals("TERM")){
        //Serial.println("Recebeu TERM");
        break;
      }
      if(state == RADIOLIB_ERR_NONE){
      
        vetor_RSSI[contador] = radio.getRSSI();
        vetor_SNR[contador] = radio.getSNR();
        contador++;
        //Serial.println(str_read);
        startMillis = millis();
       
      }
          
    }
   

  SetDefaultParam();
  
  
  
  if(contador > 10){
    msg_ult = String(getSF())+"/"+String(getBW())+"/"+String(getPT());
    v_Verifica = verificaParam(vetor_RSSI, vetor_SNR, contador);

  }else{
    String msg_volta = "OK_";
    
    msg_volta += msg_ult; 
    radio.transmit(msg_volta);
    Serial.print("Parametros ideais: ");
    Serial.println(msg_ult);
    String* valores = Parser(msg_ult);
    changeParam(valores[0].toInt(),valores[1].toFloat(),valores[2].toInt());
    Serial.println("TERM_VER");
    Serial.print("PARM_IDEAIS: ");
    Serial.println(msg_ult);
    String valor;
    while(true){
      currentMillis = millis();
      int state = radio.receive(valor);
      
      if(currentMillis-startMillis >= period){
        break;
      }
      if(state == RADIOLIB_ERR_NONE && valor.equals("TERM")){
        //Serial.println("Recebeu TERM");
        break;
      }
      if(state == RADIOLIB_ERR_NONE){
      
        Serial.println(valor);
        startMillis = millis();
       
      }
          
    }
    while(true){}
  }

  String param_atual = String(getSF())+"/"+String(getBW())+"/"+String(getPT())+"/";
  String valor;
  String msg_volta = param_atual;
  //msg_volta += param_atual; 
  radio.transmit(msg_volta);
  //Serial.print("Parametros ideais: ");
  //Serial.println(param_atual);
  String* valores = Parser(param_atual);
  changeParam(valores[0].toInt(),valores[1].toFloat(),valores[2].toInt());
  Serial.println("TERM_VER");
  Serial.print("PARM_IDEAIS: ");
  Serial.println(param_atual);
  while(true){
    currentMillis = millis();
      int state = radio.receive(valor);
  
      if(currentMillis-startMillis >= period){
        break;
      }
      if(state == RADIOLIB_ERR_NONE && valor.equals("TERM")){
        //Serial.println("Recebeu TERM");
        break;
      }
      if(state == RADIOLIB_ERR_NONE){
      
        
        Serial.println(valor);
        startMillis = millis();
      }
          
  }
  radio.transmit("TRC_OK");
  while(true){}
  
}
