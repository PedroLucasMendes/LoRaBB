#include <RadioLib.h>
#include "heltec.h"
#include <algorithm>
#define BAND    915E6
// Load Wi-Fi library
#include <WiFi.h>


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

int inicio = 0;
int fim = 107;

String msg_ult = "12/125.00/20/";

struct params_pacote{
  int SF_v;
  float BW_v;
  int PT_v;
  float R_v;
};

params_pacote vetor_Parametros[108];
float vetor_R[108] = {0.9013173297698337, 0.8752479187542421, 0.6491135690917589, 0.6704580998717395, 0.6187113484669597, 0.6826568785549645, 0.5803056881270774, 0.5332989944141053, 0.4979637777057168, 0.5001860194038614, 0.46254467846156344, 0.48816158067088833, 0.37613910159927566, 0.3744553947818161, 0.3766979785683875, 0.358591444145284, 0.36541370088089087, 0.37198230422815615, 0.739168570396819, 0.7445377050216235, 0.7278600270738429, 0.7404584899927842, 0.7087738456737223, 0.7143338320016248, 0.561021039838973, 0.5628430255547587, 0.5698577412952988, 0.5876135975224501, 0.6361986661808485, 0.5309142061458098, 0.38704511792184115, 0.38105321267164716, 0.38862253617409137, 0.38847328244274815, 0.37710664236071556, 0.38440070639170565, 0.7044484911449995, 0.707311344043143, 0.704201416581216, 0.7086898522653905, 0.7007100945653412, 0.7041958148190346, 0.6059414795871748, 0.6264518112304672, 0.6682880587940525, 0.5857700538779307, 0.5446148507980569, 0.5970310156647826, 0.392483764384186, 0.38964737381793324, 0.3824952613752887, 0.38951350119630856, 0.3860772473510311, 0.386521590520679, 0.7139234172648209, 0.7192017014165812, 0.7144729596293343, 0.713208480498272, 0.716345847100376, 0.7222172534275189, 0.6999452166647677, 0.7003902244502677, 0.7028055334016938, 0.6987926778322129, 0.7005430860962363, 0.6985070829060802, 0.45382037468057745, 0.5040376837188105, 0.5602337831711994, 0.423643062696858, 0.5413567226325002, 0.5188194755138846, 0.9102358629932238, 0.8476895893156806, 0.9304201321636094, 0.9163037864114542, 0.9242829744407732, 0.9298456192320839, 0.9148830276100414, 0.9012361854847898, 0.9147409897079488, 0.9042473357122272, 0.9148102996468042, 0.9098879647563708, 0.8918793437393187, 0.8971570582203485, 0.8770514465328115, 0.8898883445368577, 0.8826744141885989, 0.8956579696935172, 0.9184529131237775, 0.9103089195117491, 0.9026685114986588, 0.9028947383666478, 0.9028820591697998, 0.9096215487448256, 0.8984275188940792, 0.8876699517678781, 0.8822738407200639, 0.8669474193915917, 0.881363791728381, 0.8810964262656185, 0.8226537987130395, 0.8049050967185034, 0.8226113706277771, 0.7884426059259872, 0.8385678091622364, 0.8635002371193553};

void bubbleSort(params_pacote arr[], int n) {
  for (int i = 0; i < n-1; i++) {
    for (int j = 0; j < n-i-1; j++) {
      // Troca os elementos se estiverem na ordem errada
      if (arr[j].R_v > arr[j+1].R_v) {
        params_pacote temp = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = temp;
      }
    }
  }
}

void set_Params(){
  int cont = 0;
  for(int SF = 0; SF < 6; SF++){
    for(int BW = 0; BW < 3; BW++){
      for(int PT = 0; PT < 6; PT++){
        params_pacote pacote;
        pacote.SF_v = Vetor_SF[SF];
        pacote.BW_v = Vetor_BW[BW];
        pacote.PT_v = Vetor_PT[PT];
        pacote.R_v = vetor_R[cont];
        vetor_Parametros[cont] = pacote;
        cont++;
      }
    }
  }
  bubbleSort(vetor_Parametros, 108);
  
}

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
  //Serial.println("Comecando Handshake");
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
  //Serial.println("Terminou Handshake");
}

//Função de recebimento de pacote
void Receiver_SendPacket(int SF, float BW,  int PT){
  Serial.println("Comecando Recebimento de pacote");
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
      
      Serial.print(String(SF)+";"+String(BW)+";"+String(PT));
      Serial.print(";"+String(radio.getRSSI()));
      Serial.println(";"+String(radio.getSNR()));
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
       Serial.println("SEMPACTRC");
       loop = false;
    }
      //Heltec.display->clear();    
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

// Função para encontrar o valor máximo em um vetor
float encontraMaximo(float vetor[], int tamanho) {
    float maximo = vetor[0];  // Inicializa o máximo com o primeiro elemento do vetor

    for (int i = 1; i < tamanho; i++) {
        if (vetor[i] > maximo) {
            maximo = vetor[i];
        }
    }

    return maximo;
}

// Função para encontrar o valor mínimo em um vetor
float encontraMinimo(float vetor[], int tamanho) {
    float minimo = vetor[0];  // Inicializa o mínimo com o primeiro elemento do vetor

    for (int i = 1; i < tamanho; i++) {
        if (vetor[i] < minimo) {
            minimo = vetor[i];
        }
    }

    return minimo;
}

//Implementação dos algoritmos
int verificaParam(float vetor_RSSI[],float vetor_SNR[],int contador_perda){
  float somaRSSI = 0;
  float somaSNR = 0;
  int tamanho = sizeof(vetor_RSSI) / sizeof(vetor_RSSI[0]);

  for(int i = 0; i < tamanho;i++){
    somaRSSI+= vetor_RSSI[i];
    somaSNR+= vetor_SNR[i]+20;
  }
  float mediaRSSI = (somaRSSI/tamanho);
  float mediaSNR = somaSNR/tamanho;
  Serial.print(String(mediaRSSI));
  Serial.println(";"+String(mediaSNR));

  int RSSImi = encontraMinimo(vetor_RSSI,20);
  int RSSImax = encontraMaximo(vetor_RSSI,20);

  int SNRmi = encontraMinimo(vetor_SNR,20);
  int SNRmax = encontraMaximo(vetor_SNR,20);

  float RSSInorm = (mediaRSSI - RSSImi)/ (RSSImax - RSSImi);
  float SNRnorm = (mediaSNR - SNRmi)/ (SNRmax - SNRmi);
  
  float porcCt = (contador_perda*100)/20;
  porcCt = porcCt/100;
  
  float R = 0.2*(RSSInorm)+0.2*SNRnorm+0.6*porcCt;
  //Serial.println("RSSI: "+String(mediaRSSI));
  //Serial.println("SNR: "+String(mediaSNR));
  Serial.println("R atual: "+String(R));
  
  //Serial.print("R: ");
  //Serial.println(R);
  int j;
  
  for(int i = 0; i< 108;i++){
    if(vetor_Parametros[i].SF_v == valor_SF && vetor_Parametros[i].BW_v == valor_BW && vetor_Parametros[i].PT_v == valor_PT){
      Serial.println("R do dataBase: "+String(vetor_Parametros[i].R_v));
      j = i;
    }
  }
  int meio = j;
  while(inicio<=fim){
    float intervalo_ideal = R - vetor_Parametros[meio].R_v;
    if(intervalo_ideal < 0.1 && intervalo_ideal > -0.1){
      //Serial.println("0");
      break;
    }
    if(intervalo_ideal < 0){
      inicio = meio + 1;
      meio =  (int)(inicio+fim)/2;
      valor_SF = vetor_Parametros[meio].SF_v;
      valor_BW = vetor_Parametros[meio].BW_v;
      valor_PT = vetor_Parametros[meio].PT_v;
      //Serial.println("Direita");
      //Serial.println("meio: "+String(meio));
    }else{
      fim = meio-1;
      meio = (int)(inicio+fim)/2;
      valor_SF = vetor_Parametros[meio].SF_v;
      valor_BW = vetor_Parametros[meio].BW_v;
      valor_PT = vetor_Parametros[meio].PT_v;
      //Serial.println("Esquerda");
      //Serial.println("meio: "+String(meio));      
    }
    //Serial.println("inicio: "+String(inicio));
    //Serial.println("fim: "+String(fim));
  }
  //Serial.println("faz algo");
  return 0;
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

  //Wifi Server
  //Serial.println("Setting AP (Access Point)...");


  SetDefaultParam();
  Receiver_HandShake();
  set_Params();
  Serial.println("COM_VER");
  
  

}

void loop() {

  String str_read;
  int contador = 0;
  unsigned long period = 1000*60;
  
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
        Serial.println(str_read);
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
    String* valores = Parser(msg_ult);
    changeParam(valores[0].toInt(),valores[1].toFloat(),valores[2].toInt());
    Serial.println("TERM_VER");
    Serial.print("PARM_IDEAIS: ");
    Serial.println(msg_ult);
    String valor;
    startMillis = millis();
    while(true){
      currentMillis = millis();
      int state = radio.receive(valor);
  
      if(currentMillis-startMillis >= period){
        break;
      }
      if(state == RADIOLIB_ERR_NONE && str_read.equals("TERM")){
        //Serial.println("Recebeu TERM");
        break;
      }
      if(state == RADIOLIB_ERR_NONE){
        startMillis = millis();
        Serial.println(valor);
       
      }
          
    }
    while(true){}
  }

  String param_atual = String(getSF())+"/"+String(getBW())+"/"+String(getPT())+"/";
  
  Serial.print("Trocando para os parâmetros");
    Serial.println(param_atual);
  if(v_Verifica == 1){
    radio.transmit(param_atual);
    String* valores = Parser(param_atual);
    changeParam(valores[0].toInt(),valores[1].toFloat(),valores[2].toInt());
    
  }else{
    radio.transmit("TRC_OK");
    Serial.println("TERM_VER");
    Serial.print("PARM_IDEAIS: ");
    Serial.println(param_atual);
    String valor;
    startMillis = millis();
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
    while(true);
  }
  
}
