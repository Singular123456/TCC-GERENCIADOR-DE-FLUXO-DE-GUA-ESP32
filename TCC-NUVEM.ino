int LED_FLUXO = 27; //Pino do led do fluxo
int LED_BOIA = 12;  //Pino do led da boia
int RELE = 13;      //Pino do rele 
int CAIXA = 14;     //Pino da boia 
int FLUXO = 35;     //Pino do sensor de fluxo   
int contaPulso;     //Variável para a quantidade de pulsos
int i=0;            //Variável para contagem
int contador_c = 0; //Contador para registro da hora do estado_c 
int contador_b = 0; //Contador para registro da hora do estado_b
float vazao;        //Variável para armazenar o valor em L/min
float media=0;      //Variável para tirar a média a cada 1 minuto
char estado = 'd';  //Variável inicialmente no estado d

// ID do modelo, nome do dispositivo e token de autenticação são fornecidos pelo Blynk.Cloud

#define BLYNK_TEMPLATE_ID "TMPL2XmeBGJU"
#define BLYNK_DEVICE_NAME "Projeto monitoramento de fluxo de água teste"
#define BLYNK_AUTH_TOKEN "yWRkZiCT72CH5ePtYq6sBL4WWpTpGVL4"

//Declaração de bibliotecas

#include <Blynk.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

char auth[] = BLYNK_AUTH_TOKEN;      //Autenticação de token no blynk
char ssid[] = "REI DA NET DAYVISON"; //Nome da rede wifi 
char pass[] = "Singular1";           //Senha da rede wifi

WidgetLED led_bomba(V0);     //Declarando objeto led bomba no pino virtual 0  
WidgetLED led_fluxo(V1);     //Declarando objeto led fluxo no pino virtual 1  
WidgetLED led_boia(V2);      //Declarando objeto led boia no pino virtual 2 
WidgetRTC rtc;               //Declarando objeto rtc (relogio com sincronismo na rede)
BlynkTimer timer;            //Declarando objeto timer

//Esta função é chamada toda vez que o dispositivo é conectado ao Blynk.Cloud

BLYNK_CONNECTED()

{
  //Altere a mensagem do botão Web Link para "Parabéns!"
  
  Blynk.setProperty(V8, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V8, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V8, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");

  //Inicializando o sincronismo do rtc com a nuvem 
  rtc.begin(); 
}

void setup() 

{
Serial.begin(115200); //iniciando terminal com baud rate de 115200

pinMode(LED_FLUXO,OUTPUT);  //Definindo o led do fluxo como saida
pinMode(LED_BOIA,OUTPUT);   //Definindo o led da boia como saida
pinMode(RELE,OUTPUT);       //Definindo o relé como saida
pinMode(CAIXA,INPUT);       //Definindo a boia como entrada
pinMode(FLUXO,INPUT);       //Definindo o fluxo como entrada

attachInterrupt(digitalPinToInterrupt(35),incpulso,RISING); //Configura o pino 35 como interrupção 

Serial.println("\n\nInicio\n\n"); //Imprime Inicio na serial
Blynk.begin(auth, ssid, pass);    //Inicia o blynk com os valores acima setados
setSyncInterval(10 * 60);         //Intervalo de sincronização em segundos (10 minutos)
timer.setInterval(500L,gerencia_fluxo); 
}






void loop()

{

//execução do BLYNK
Blynk.run();
timer.run();

//Atualização de data nos pinos virtuais

String currentDate = String(day()) + "/" + month() + "/" + year();
Blynk.virtualWrite(V4,currentDate);
Blynk.virtualWrite(V5,"Caixa está vázia!");
Blynk.virtualWrite(V10,"Caixa encheu!");
Blynk.virtualWrite(V11,"Chegou água da rua!");
Blynk.virtualWrite(V12,"Não tem mais água na rua!");


}

void gerencia_fluxo()
{

int BOIA = digitalRead(CAIXA);

//Atribuição das horas as variáveis 

String currentTime = String(hour()) + ":" + minute() + ":" + second();
String currentTime1 = String(hour()) + ":" + minute() + ":" + second();
String currentTime2 = String(hour()) + ":" + minute() + ":" + second();
String currentTime3 = String(hour()) + ":" + minute() + ":" + second();


  switch(estado) 
  
     {

      
      //ESTADO AGUA NA RUA E CAIXA VAZIA  
     
      case 'a':
      
      if(contaPulso > 1 && BOIA==LOW)
      {
      Serial.println("Tem água na rua e nível de água está baixo,ligando bomba!"); 
      Serial.println("Bomba Ligada,enchendo a caixa!");
      digitalWrite(LED_FLUXO,HIGH); //Liga a led física do fluxo
      digitalWrite(RELE,HIGH);      //Liga a bomba
      led_fluxo.on();               //Liga a led virtual do fluxo
      led_bomba.on();               //Liga a led virtual da bomba
      digitalWrite(LED_BOIA,HIGH);  //Liga a led física da boia
      led_boia.on();                //Liga a led virtual da boia
      delay(2000);
      led_boia.off();               //Desliga a led virtual da boia
      digitalWrite(LED_BOIA,LOW);   //Desliga a led física da boia
      fluxo_agua();                 //Chama a função para calcular a vazao de agua
      estado = 'd';
      }

      break;

     //ESTADO AGUA NA RUA E CAIXA CHEIA
     
     case 'b':

     if(contador_b == 0)
     
     {
     Blynk.virtualWrite(V9,currentTime); //Registra a hora que chegou agua da rua 
     contador_b = contador_b + 1;
     }
    
     digitalWrite(LED_BOIA,LOW);   //Desliga led fisico da boia
     digitalWrite(LED_FLUXO,HIGH); //Liga led fisico do fluxo
     led_fluxo.on();               //Liga led virtual do fluxo
     delay(2000);
     digitalWrite(LED_FLUXO,LOW);  //Desliga led fisico do fluxo
     led_fluxo.off();              //Desliga led virtual do fluxo
     led_bomba.off();              //Desliga led virtual da bomba
     led_boia.off();               //Desliga led virtual da boia
     fluxo_agua();                 //Chama a função para calcular a vazao de agua
     estado = 'd';
     break;

     case 'c':

     //ESTADO SEM AGUA NA RUA E CAIXA VAZIA

     if(contador_c == 0)
     
     {
     Blynk.virtualWrite(V6,currentTime1); 
     contador_c = contador_c + 1;
     }
     digitalWrite(LED_FLUXO,LOW); //Desliga o led fisico do fluxo
     Serial.println("Nível de água baixo,mas sem água na rua!"); 
     delay(1000); 
     digitalWrite(RELE,LOW);      //Desligar a bomba 
     digitalWrite(LED_BOIA,HIGH); //Liga a led fisica da boia
     led_boia.on();               //Liga led virtual da boia
     led_fluxo.off();             //Desliga o led virtual do fluxo
     led_bomba.off();             //Desliga o led virtual da bomba
     estado = 'd';
     break;

      //Estado inicial
        case 'd':


        Serial.println("INICIO DE ESTADOS");
        
                
        //TRANSICAO HORA FLUXO    
            
        if(contaPulso < 1 && contador_b > 0) // Se não há água na rua e o contador de b passou de 0 
        {
          
          digitalWrite(LED_FLUXO,LOW); //Desliga a led física do fluxo
          led_fluxo.off();             //Desliga a led virtual do fluxo
          
          if(contador_b < 2)
          {
          Blynk.virtualWrite(V8,currentTime2); //Registra a hora que não tem mais água da rua
          contador_b = contador_b + 1;
          }
        }


       //TRANSICAO HORA BOIA
        
        if(BOIA==HIGH && contador_c > 0)
        {
          if(contador_c < 2)
          {
          Blynk.virtualWrite(V7,currentTime3);
          contador_c = contador_c + 1;
          }
        }

        //TRANSICAO HORA BOMBA
        
        if(BOIA==HIGH && contaPulso < 1)
        
        {
          
          digitalWrite(LED_BOIA,LOW);  //Desligar a led física da boia
          digitalWrite(RELE,LOW);      //Desligar a bomba 
          digitalWrite(LED_FLUXO,LOW); //Desligar a led física do fluxo
          led_boia.off();              //Desligar a led virtual da boia
          led_bomba.off();             //Desligar a led virtual da bomba
          led_fluxo.off();             //Desligar a led virtual do fluxo
     
        }
      
      //TRANSIÇÕES DE ESTADO
  
      if(contaPulso < 1 && BOIA==HIGH) //ESTADO SEM AGUA NA RUA E CAIXA CHEIA 
      
      {estado = 'd';}

      if(contaPulso > 1 && BOIA==LOW) //ESTADO AGUA NA RUA E CAIXA VAZIA
      
      {estado = 'a';}

      if(contaPulso > 1 && BOIA==HIGH) //ESTADO AGUA NA RUA E CAIXA CHEIA
      
      {estado = 'b';}
     
      if(contaPulso < 1 && BOIA==LOW) //ESTADO SEM AGUA NA RUA E CAIXA VAZIA
      
      {estado = 'c';}
     
     }    
  
  }

  

void fluxo_agua()

{
  contaPulso = 0;   //Zera a variável para contar os giros por segundos
  sei();            //Habilita interrupção
  delay(1000);      //Aguarda 1 segundo
  cli();            //Desabilita interrupção
  
  vazao = contaPulso / 5.5; //Converte para L/min
  media=media+vazao;        //Soma a vazão para o calculo da media
  i++;
  
  Serial.print(vazao);       //Imprime na serial o valor da vazão
  Serial.print(" L/min - "); //Imprime L/min
  Serial.print(i);           //Imprime a contagem i (segundos)
  Serial.println("s");       //Imprime s indicando que está em segundos
  Blynk.virtualWrite(V3,vazao); //Coloca o valor da vazao no pino virtual 3
 }

 void incpulso ()
{ 
 contaPulso++; //soma o valor do sensor de pulso a cada volta dada
} 
