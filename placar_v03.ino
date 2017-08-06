/*--------------------------------------------------------------------------------------
 
 placar_v03.cpp 
 19/5/2013
 - Incluido Primeiro e Segundo Tempo
 - Arrumado novamente o letreiro, para poder apertar botao durante a mensagem
 - 
 
 
 Placar Eletronico para Futebol de Mesa
 Copyright (C) 2021 Marcos Hemann (marcos <at> mhemann <dot> com <dot> br)
 
 
 Função do Teclado
 S1(V) Dec placar jogador A
 S2(^) Inc placar jogador A
 S3(V) Dec placar jogador B
 S4(^) Inc placar jogador B
 
 S1+S3 (V) Zera cronometro
 S2+S4 (^) Dispara e PAra cronometro
 
 Ou outro botao na interrupção para fazer o Start/Stop/Clean do Cronometro
 
 --------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------
 Includes
 --------------------------------------------------------------------------------------*/
#include <SPI.h>        // Biblioteca do Serial Pin Interface para medir a Temperatura e Humidade
#include <DMD.h>        // Bib do Painel
#include <TimerOne.h>   // Bib de temporizadores
#include <Bounce.h>    // Bib Debounce botoes
#include "DHT.h" // Bib do Termometro e Higrometro

#include "SystemFont5x7.h" // Fonte do Painel
#include "Arial_black_16.h" // Fonte do Painel
#include "Arial14.h"   // Fonte do painel

#include "floatToString.h" // bib MH mat str

// Define dados do jogo

#define JA  "T-A"  // Iniciais do Jogador A
#define JB  "T-B"  // Iniciais do Jogador B

#define MSG1 "Futmesa: o esporte fair-play      Bem-vindo `a PHOENIX ARENA     Desejamos um bom jogo"   //Mensagem 
char * MI1T = "Inicio do primeiro tempo";
char * MI2T = "Inicio do segundo tempo";
char * MF1T = "Final do primeiro tempo";
char * MF2T = "Final do segundo tempo";
char * MMM;

int SA = 0;    // Score inicial do Jogador A
int SB = 0;   //  Score inicial do Jogador B
int csecond=0, cminute=25, chour=0; // declara as Constantes do Tempo de jogo

int second=csecond, minute=cminute, hour=chour; // declara as variaveis do Tempo de jogo
int CRON = 4;  // 0- Jogo pausado, 1 - Jogando normal, 2 Recomeçar partida, carregar novamente tempos, 3 -- Partida finalizada, 4 - Partida iniciando
int TJ=1;  // Tempo do Jogo 1 - Primeiro Tempo, 2 - Segundo tempo
//int mostralet = 0;  // Mostra Letreiro   0 - Nao mostrando, 1 = Mostrando
// Definicao dos Pinos
int pinoBuzzer = 4;//pino que o buzzer e conectado.
int pinoBot = 3 ; // pino do botao
#define BUTTON 3
int sitBot = 0;  // 0 botao nao acionado, 1 - bot acionado, 2 esperando conformar Bot acionado, 3 esperando confirmar bot nao acionado
#define DHTPIN 2  // pino do modulo DTH TEm e Hum

//Inicializa das Bibliotecas 
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
#define DHTTYPE DHT22 

int ESTADO = 0;   // Estado inicial
long TEMPO = 0;   // Tempo decorrido da ultima mudança de estado
byte tocaBuzzer = 0; // 0 - Nao toca buzzer, 1 - Toca buzzer, 2 - buzzer tocando
int tempoBuzzer = 0; // Tempo que a buzzer vai tocar
long timerBuzzer = 0;  // Timer do Buzzer diz quando começou a tocar
int b ;
int buttonState;             // Leitura atual do pino de entrada
int lastButtonState = LOW;   // Leitura anterior do pino de entrada
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // O tempo de  debounce ; Aumente se a saida balancear
long bsoma = 0;
long bcont = 1;
long bmed = 0;
int buttonValue;                      
byte buttonAct = 0; // flag, segue a pressao do botao
byte buttonPressed = 0;  // Que botao foi pressionado
byte lastButtonPressed = 0; // Previo botao pressionado

// Varaiveis mostra Frase
boolean ret=false;   // Flag de mostra frase ja mostrou todas letras
long start=millis();   // Momento que 
long timer=start;
int mostralet = 0;   // Flag 0 Nao esta mostrand Letreiro, 1 - Esta mostrando letreiro
//Inicializa 
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
DHT dht(DHTPIN, DHTTYPE);
Bounce bouncer = Bounce( BUTTON,50 );  // Inicializa debounce botao

/*--------------------------------------------------------------------------------------
 Atendimento da Interrupcao do Timer1 (TimerOne) chamado pelo periodo definido 
 em  Timer1.initialize();
 --------------------------------------------------------------------------------------*/
void ScanDMD()
{ 
  dmd.scanDisplayBySPI();  // Faz o refresh do display 5ms
  countdown();        // Atualiza o  o relogio
}

/*--------------------------------------------------------------------------------------
 setup
 Chamado pela arquitetura  Arduino antes de iniciar o loop principal
 --------------------------------------------------------------------------------------*/
void setup(void)
{
  //inicializa Timer1 interrupção da CPU usada para scan e refresh do display
  Timer1.initialize( 4000 );           // Periodo em microseconds para chamar ScanDMD. Mais que 5000 (5ms) vamos notar o flicker.
  Timer1.attachInterrupt( ScanDMD );   // A Interrupcao do Timer1 e' associada ao ScanDMD que chama o dmd.scanDisplayBySPI()
  dmd.clearScreen( true );   // TRUE todos pixels OFF, FALSE todos pixels ON
  dht.begin();   // Inicializa o sensor.
  pinMode(pinoBuzzer, OUTPUT); // Configura o Pino Buzzer como Saida
  pinMode(pinoBot, INPUT); // Configura o Pino Botao como Entrada
  //  digitalWrite(pinoBot, HIGH);       // ativa o resistor de pullup
  digitalWrite(pinoBuzzer, LOW);   // Desliga Buzzer
  dmd.selectFont(System5x7);
  for( b = 0 ; b < 20 ; b++ )
   {
      dmd.drawTestPattern( (b&1)+PATTERN_STRIPE_0 );
      delay( 200 );      
   }
  dmd.clearScreen( true );  
  Serial.begin(9600);
  TEMPO=millis();   // Inicia contagem de tempo
  ESTADO=0;
  //  mostra_frase();
}

/*--------------------------------------------------------------------------------------
 loop
 Loop Principal do Arduino 
 --------------------------------------------------------------------------------------*/
void loop(void)
{
  // Funções permanentes

  // Testa se o botao foi pressionado
  // CRON - 0- Jogo pausado, 1 - Jogando normal, 2 REcomeçar partida, carregar novamente tempos, 3 -- Partida finalizada, 4 - Partida iniciando
  if ( bouncer.update() ) {
    if ( bouncer.read() == HIGH) {
      TEMPO = millis();
      ESTADO=0; 
      dmd.clearScreen( true );
      tocaBuzzer=1;
      tempoBuzzer=300;
      if (CRON == 1){ // relogio parado passa para ativo
        CRON = 0;
      }
      else if (CRON == 0){ // Rel ativo passa para parado
        CRON = 1;
      }
      else if (CRON == 4){ // Rel ativo passa para parado
        CRON = 1;
      }
      else if (CRON == 3){ // ja houve fim de partida
        CRON = 4;
        second=csecond;
        minute=cminute;
        hour=chour;
        if (TJ==1){    //Final do primeiro tempo
          TJ=TJ+1;
        }
        else if (TJ==2){
          SA =0 ;
          SB = 0;
          TJ=1;
        } 
      }
    }
  }  // Fim le botao
  // Inicio LE treclado
  if( analogRead(0)>30){ // tec foi pressionado
    le_tec();
  }
  else{
    buttonPressed = 0; //  NAO Pressionado
    buttonAct = 0;
  }
  // Fim Le trclado
  // Inicio Buzzer
  if (tocaBuzzer==1 ){
    digitalWrite(pinoBuzzer, HIGH);   // Liga Toca Buzzer
    tocaBuzzer=2;
    timerBuzzer = millis();  // marca o inicio do tempo de toque
  }
  if (tocaBuzzer==2){    
    if ((millis() - timerBuzzer) > tempoBuzzer) {
      digitalWrite(pinoBuzzer, LOW);   // Desliga Toca Buzzer quando ele toca o tempo definido por tempoBuzzer
      tocaBuzzer=0;    
    }
  }
  // fim buzzer

  // DEBUG
  /*
  if (buttonValue<10){
   bcont=1;
   bsoma=0;
   bmed=0;
   }
   bsoma=bsoma+buttonValue;
   bcont=bcont+1;
   bmed=bsoma / bcont;
   */
  Serial.print("BOT=");
  Serial.print(buttonValue);
  Serial.print("/CRON=");
  Serial.print(CRON);
  Serial.print("/ESTADO="); 
  Serial.print(ESTADO);
  Serial.print("/TEC="); 
  Serial.print(buttonPressed);
    Serial.print("/ML="); 
  Serial.println(mostralet);

  /*
  Serial.print("/SOMA=");
   Serial.print(bsoma);
   Serial.print("/CONT=");
   Serial.print(bcont);
   Serial.print("/Media=");
   Serial.println(bmed); 
   */

  // FIM do DEbug

  // Mostra jogo parado se CRON=0

  // Maquina de Estado
  switch (ESTADO){
  case 0:
    mostra_relogio();
    if ((millis() - TEMPO) > 3000) {
      ESTADO=6;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
      dmd.clearScreen( true );
    }
    break;
  case 1:
    mostra_placar();
    if ((millis() - TEMPO) > 5000) {
      ESTADO=2;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
      dmd.clearScreen( true );
    }
    break;
  case 2:
    mostra_tempo();  
    if ((millis() - TEMPO) > 3000) {
      ESTADO=3;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
      dmd.clearScreen( true );
    }
    break;
  case 3:
    mostra_humidade(); 
    if ((millis() - TEMPO) > 3000) {
      ESTADO=4;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
      dmd.clearScreen( true );
    }
    break;
  case 4:
    mostra_temp();  
    if ((millis() - TEMPO) > 3000) {
      ESTADO=5;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
      dmd.clearScreen( true );
    }
    break;
  case 5:
    if (CRON == 0 ) { // Jogo Pausado ok
       if(mostra_frase("Jogo Parado" )) {
        ESTADO=7;  //muda de estado para o proxima
        TEMPO=millis(); //zera o tempo
        dmd.clearScreen( true );
      }  
    }else {
      ESTADO=7;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
    }
    break;
  case 6: // inicio jogo ok
    if (CRON == 4 ) {
  //    char * MMM;
      if(TJ==1){
        MMM = MI1T;
      }else{
        MMM = MI2T;
      }  
       if(mostra_frase(MMM)) {
        ESTADO=1;  //muda de estado para o proxima
        TEMPO=millis(); //zera o tempo
        dmd.clearScreen( true );
      }  
    }else {
      ESTADO=1;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
    }
    break;
  case 7:  // Fim de jogo OK
    if (CRON == 3 ) {
      if(TJ==1){
        MMM = MF1T;
      }else{
        MMM = MF2T;
      }
      if(mostra_frase(MMM)) {
        ESTADO=8;  //muda de estado para o proxima
        TEMPO=millis(); //zera o tempo
        dmd.clearScreen( true );
      }   
    }
    else{
      ESTADO=8;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo 
    }  
    break;  
  case 8:  // Mostra Frase - Se nao esta inicia jogo
    if (CRON == 4 ){
      if(mostra_frase(MSG1)) {
        ESTADO=0;  //muda de estado para o proxima
        TEMPO=millis(); //zera o tempo
        dmd.clearScreen( true );
      }  
    }else {
      ESTADO=0;  //muda de estado para o proxima
      TEMPO=millis(); //zera o tempo
    }
    break;   
  }
}
// FIM Loop principal
// Inicio Rotinas
void le_tec(){
  lastButtonPressed = buttonPressed;
  buttonValue = analogRead(0);   // read the input pin 0

    if(buttonValue >= 805 and buttonValue <= 825 and  buttonAct == 0) //analog value 814
  {
    buttonPressed = 1;//   VV button
    buttonAct = 1;
    SA=0;
    SB=0;
    TEMPO = millis();
    ESTADO=1; 
    dmd.clearScreen( true );

  }
  else if(buttonValue >= 710 and buttonValue <= 725 and  buttonAct == 0) //analog value 718
  {
    buttonPressed = 2; // ->^ button
    buttonAct = 1;
    SB++;
    TEMPO = millis();
    ESTADO=1; 
    dmd.clearScreen( true );
  }
  else if(buttonValue >= 660 and buttonValue <= 675 and  buttonAct == 0) //analog value 667
  {
    buttonPressed = 3; //  ->V button
    buttonAct = 1;
    if(SB>0){
      SB--;
      TEMPO = millis();
      ESTADO=1; 
      dmd.clearScreen( true );
    }  
  }

  else if(buttonValue >= 600 and buttonValue <= 616 and  buttonAct == 0) //analog value 609
  {
    buttonPressed = 4; //  ^<- button
    buttonAct = 1;
    SA++;
    TEMPO = millis();
    ESTADO=1; 
    dmd.clearScreen( true );

  }
  else if(buttonValue >= 505 and buttonValue <= 540 and  buttonAct == 0) //analog value 532
  {
    buttonPressed = 5; //  V<- button
    buttonAct = 1;
    if(SA>0){
      SA--;
      TEMPO = millis();
      ESTADO=1; 
      dmd.clearScreen( true );
    }
  }
  lastButtonPressed = buttonPressed;   // guarda ultimo pressionado 

}
void mostra_parado(){
  //  dmd.clearScreen( true );
  dmd.selectFont(System5x7);

  dmd.drawString(  6,  0, "Jogo", 4, GRAPHICS_NORMAL );
  dmd.drawString(  2,  8, "Pausa", 5, GRAPHICS_NORMAL );
}
void mostra_ini_jogo(){
  //  dmd.clearScreen( true );
  dmd.selectFont(System5x7);

  dmd.drawString(  6,  0, "Jogo", 4, GRAPHICS_NORMAL );
  dmd.drawString(  0,  8, "Inicia", 6, GRAPHICS_NORMAL );
}
void mostra_fim_jogo(){
  //  dmd.clearScreen( true );
  dmd.selectFont(System5x7);

  dmd.drawString(  6,  0, "Jogo", 4, GRAPHICS_NORMAL );
  dmd.drawString(  8,  8, "FIM", 3, GRAPHICS_NORMAL );
}
void mostra_tempo(){
  //  dmd.clearScreen( true );
  dmd.selectFont(System5x7);
  char* temp;
  char buffer[25];
  temp = floatToString(buffer,TJ,1,0,true);
   dmd.drawString(  17,  -4, ".", 1, GRAPHICS_NORMAL );
  dmd.drawString(  12,  0, temp, 1, GRAPHICS_NORMAL );
  dmd.drawString(  1,  8, "TEMPO", 5, GRAPHICS_NORMAL );
}
void mostra_bot(){
  float  b =  analogRead(0);
  char* rbot;
  char bufferb[25];
  rbot = floatToString(bufferb,b,3,0,true);
  //  dmd.selectFont(System5x7);
  //  dmd.clearScreen( true );
  // Temperatura
  dmd.drawString(  0,  8, rbot, 4, GRAPHICS_NORMAL );

}
void mostra_temp(){
  float  t = dht.readTemperature();
  char* temp;
  char buffer[25];
  temp = floatToString(buffer,t,3,0,true);
  dmd.selectFont(System5x7);
  // dmd.clearScreen( true );
  // Temperatura
  dmd.drawString(  23,  4, ".", 1, GRAPHICS_NORMAL );
  dmd.drawString(  4,  0, "Temp", 4, GRAPHICS_NORMAL );
  dmd.drawString(  0,  8, temp , 4, GRAPHICS_NORMAL );
  dmd.drawString(  27,  8, "C", 1, GRAPHICS_NORMAL );  
}
void mostra_humidade(){
  float h = dht.readHumidity();
  char* humi;
  char buffer2[25];
  humi = floatToString(buffer2,h,3,0,true);
  dmd.selectFont(System5x7);
  //  dmd.clearScreen( true );
  // Mostra Umidade
  dmd.drawString(  4,  0, "Umid", 4, GRAPHICS_NORMAL );
  dmd.drawString(  0,  8, humi , 4, GRAPHICS_NORMAL );
  dmd.drawString(  25,  8, "%", 1, GRAPHICS_NORMAL );
}

void mostra_placar(){
  //  dmd.clearScreen( true );
  dmd.selectFont(System5x7);
  char buffer3[25];
  char buffer4[25];
  dmd.drawString(  0,  0, JA, 3, GRAPHICS_NORMAL );
  dmd.drawString(  0,  8, JB, 3, GRAPHICS_NORMAL );
  //  for (int i = 0; i < 1000; i++) {
  int desl = 0;
  if (SA < 10){
    desl=3;
  }
  else{
    desl=0;
  }
  dmd.drawString(  21 + desl,  0,  floatToString(buffer3,SA,0,0,true), 2, GRAPHICS_NORMAL );
  if (SB < 10) {
    desl=3;
  }
  else{
    desl=0;
  }
  dmd.drawString(  21 + desl,  8,  floatToString(buffer3,SB,0,0,false), 2, GRAPHICS_NORMAL );
  //  } 

}
boolean  mostra_frase( char * MSG){
     if (mostralet == 0) {
        dmd.selectFont(Arial_Black_16);
      //  dmd.drawMarquee("Futmesa: o esporte fair-play      Bem-vindo `a PHOENIX ARENA     Desejamos um bom jogo",86,(32*DISPLAYS_ACROSS)-1,0);

        dmd.drawMarquee(MSG,strlen(MSG), (32*DISPLAYS_ACROSS)-1,0);
        mostralet = 1;
        start=millis();   // Momento que 
        timer=start;
        ret=false;  
        return false;
      }
      else{
        if (!ret){
          if ((timer+20) < millis()) {
             dmd.selectFont(Arial_Black_16);
            ret=dmd.stepMarquee(-1,0);
            timer=millis();
          }
           return false;
        }else{
       //   ESTADO=0;  //muda de estado para o proxima
          TEMPO=millis(); //zera o tempo
          mostralet = 0;
          return true;
        } 
      }
}
void mostra_relogio(){
  char bufferm[25];
  char buffers[25];
  dmd.selectFont(System5x7);
  dmd.drawString(  3,  4,  floatToString(bufferm,minute,0,3,true), 3, GRAPHICS_NORMAL );
  dmd.drawString(  18, 4,  floatToString(buffers,second,0,3,true) , 3, GRAPHICS_NORMAL );
  dmd.drawChar( 14,  4, ':', GRAPHICS_OR     );   // clock colon overlay on
}
/***********************************************************
 * Modulo Timer Decrescente                                 *
 *                 countdown()                              *
 ************************************************************/
void countdown(){

  static unsigned long lastTick = 0; // set up a local variable to hold the last time we decremented one second
  // (static variables are initialized once and keep their values between function calls)
  if (minute ==0  && CRON==1 && tocaBuzzer==0 && second==0){   // Se for ultimo minuto de jogo 
    tocaBuzzer=1;
    tempoBuzzer=50;
  } 
  if (minute ==0  && CRON==1 && tocaBuzzer==0 && second>0){   // Se for ultimo minuto de jogo 
    ESTADO=0;
  } 
  if (CRON == 1){    // coronometro nao esta parado
    // decrement one second every 1000 milliseconds
    if (second > -1) {
      if (millis() - lastTick >= 1000) {
        lastTick = millis();
        second--;
        //          serialOutput();
      }
    }

    // decrement one minute every 60 seconds
    if (minute > -1) {
      if (second < 0) {
        minute--;
        if (minute<0){  // entao chegou em zero
          second=0;
          minute=0;
          CRON=3;   // fim de jogo
          tocaBuzzer=1;
          tempoBuzzer=3000;
        }
        else{
          second = 59; // reset seconds to 60
        }
      }
    }

    // decrement one hour every 60 minutes
    if (hour > -1) {
      if (minute < 0) {
        minute = 0;  //  deixa o minuto em 0
        // pode aqui tocar alarme *******
        //hour--;
        //minute = 59; // reset minutes to 60
      }//closes if
    }//closes if
  }

} //close countdown();


































