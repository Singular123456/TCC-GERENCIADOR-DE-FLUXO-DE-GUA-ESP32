const int FLUXO = 35; 
const int CAIXA = 14; 
int RELE = 13; 
int contaPulso; //Variável para a quantidade de pulsos
int i=0; //Variável para contagem
float vazao; //Variável para armazenar o valor em L/min
float media=0; //Variável para tirar a média a cada 1 minuto
int LED_FLUXO = 27;
int LED_BOIA = 12;
int contador_c = 0;
int contador_b = 0;
char estado = 'd';

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPL2XmeBGJU"
#define BLYNK_DEVICE_NAME "Projeto monitoramento de fluxo de água teste"
#define BLYNK_AUTH_TOKEN "yWRkZiCT72CH5ePtYq6sBL4WWpTpGVL4"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <Blynk.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "REI DA NET DAYVISON";
char pass[] = "Singular1";

WidgetLED led_bomba(V0);
WidgetLED led_fluxo(V1);
WidgetLED led_boia(V2);
WidgetRTC rtc;
BlynkTimer timer;

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()

{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V8, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V8, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V8, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
  rtc.begin();

}



void setup() 
{

Serial.begin(115200);
pinMode(13,OUTPUT);
pinMode(14,INPUT);
pinMode(35,INPUT);
pinMode(LED_FLUXO,OUTPUT);
pinMode(LED_BOIA,OUTPUT);
attachInterrupt(digitalPinToInterrupt(35),incpulso,RISING); //Configura o pino 35 para trabalhar como interrupção 
Serial.println("\n\nInicio\n\n"); //Imprime Inicio na serial
Blynk.begin(auth, ssid, pass);
setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
//timer.setInterval(500L,gerencia_fluxo); 


}

void loop()

{

//execução do BLYNK
Blynk.run();
timer.run();
//Atualização de data
String currentDate = String(day()) + "/" + month() + "/" + year();
Blynk.virtualWrite(V4,currentDate);
Blynk.virtualWrite(V5,"Caixa está vázia!");
Blynk.virtualWrite(V10,"Caixa encheu!");
Blynk.virtualWrite(V11,"Chegou água da rua!");
Blynk.virtualWrite(V12,"Não tem mais água na rua!");
int BOIA = digitalRead(CAIXA);
String currentTime = String(hour()) + ":" + minute() + ":" + second();
String currentTime1 = String(hour()) + ":" + minute() + ":" + second();
String currentTime2 = String(hour()) + ":" + minute() + ":" + second();
String currentTime3 = String(hour()) + ":" + minute() + ":" + second();
 

  switch(estado) 
  
     {
      
    case 'a':
    
      //ESTADO AGUA NA RUA E CAIXA VAZIA
      if(contaPulso > 1 && BOIA==LOW)
      {
      Serial.println("Tem água na rua e nível de água está baixo,ligando bomba!"); 
      Serial.println("Bomba Ligada,enchendo a caixa!");
      digitalWrite(LED_FLUXO,HIGH); //LIGAR LED FISICO Do FLUXO
      digitalWrite(RELE,HIGH); //Ligar a bomba e a led fisica
      led_fluxo.on();
      led_bomba.on();
      digitalWrite(LED_BOIA,HIGH); //LIGAR LED FISICO DA BOIA 
      led_boia.on();
      delay(2000);
      led_boia.off();
      digitalWrite(LED_BOIA,LOW); //DESLIGAR LED FISICO DA BOIA 
      fluxo_agua();
      estado = 'd';
      }

      break;

    case 'b':

     //ESTADO AGUA NA RUA E CAIXA CHEIA

     if(contador_b == 0)
     {
     Blynk.virtualWrite(V9,currentTime);
     contador_b = contador_b + 1;
     }
     digitalWrite(RELE,LOW); // DESLIGAR O RELE
     digitalWrite(LED_BOIA,LOW); //LIGAR LED FISICO DA BOIA
     digitalWrite(LED_FLUXO,HIGH); //LIGAR LED FISICO Do FLUXO 
     led_fluxo.on();
     delay(2000);
     digitalWrite(LED_FLUXO,LOW); //LIGAR LED FISICO Do FLUXO 
     led_fluxo.off();
     led_bomba.off();
     led_boia.off();
     fluxo_agua();
     estado = 'd';
     break;

     case 'c':

     //ESTADO SEM AGUA NA RUA E CAIXA VAZIA

     if(contador_c == 0)
     
     {
     Blynk.virtualWrite(V6,currentTime1); 
     contador_c = contador_c + 1;
     }
     digitalWrite(LED_FLUXO,LOW); //LIGAR LED FISICO DA BOIA
     Serial.println("Nível de água baixo,mas sem água na rua!"); 
     delay(1000); 
     digitalWrite(RELE,LOW); //Desligar a bomba 
     digitalWrite(LED_BOIA,HIGH); //LIGAR LED FISICO DA BOIA
     led_boia.on();
     led_fluxo.off();
     led_bomba.off(); 
     estado = 'd';
     break;
      
      case 'd':


      Serial.println("INICIO DE ESTADOS");
      //TRANSICOES DAS HORAS
        
                
        //TRANSICAO HORA FLUXO    
            
        if(contaPulso < 1 && contador_b > 0)
        {
          
          digitalWrite(LED_FLUXO,LOW); //LIGAR LED FISICO DA BOIA
          led_fluxo.off();
          
          if(contador_b < 2)
          {
          Blynk.virtualWrite(V8,currentTime2);
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
          
          digitalWrite(LED_BOIA,LOW); //LIGAR LED FISICO DA BOIA
          digitalWrite(RELE,LOW); //Ligar a bomba e a led fisica
          digitalWrite(LED_FLUXO,LOW); //Ligar a bomba e a led fisica
          led_boia.off();
          led_bomba.off();
          led_fluxo.off();
     
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

BLYNK_WRITE(V14) // Executes when the value of virtual pin 0 changes
{
 while(param.asInt() == 1)
  {
    digitalWrite(RELE,HIGH);
    led_bomba.on(); 
  }
  if(param.asInt() == 0)
  {
    digitalWrite(RELE,LOW);  // Set digital pin 2 LOW 
    led_bomba.off();   
  }
 }



void fluxo_agua()

{
  contaPulso = 0;   //Zera a variável para contar os giros por segundos
  sei();      //Habilita interrupção
  delay(1000); //Aguarda 1 segundo
  cli();      //Desabilita interrupção
  
  vazao = contaPulso / 5.5; //Converte para L/min
  media=media+vazao; //Soma a vazão para o calculo da media
  i++;
  
  Serial.print(vazao); //Imprime na serial o valor da vazão
  Serial.print(" L/min - "); //Imprime L/min
  Serial.print(i); //Imprime a contagem i (segundos)
  Serial.println("s"); //Imprime s indicando que está em segundos
  Blynk.virtualWrite(V3,vazao);
 }

 void incpulso ()
{ 
 contaPulso++;
} 
