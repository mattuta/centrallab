/*
   Autor: Matheus de Souza Silva, Data: Dezembro de 2018
   revisao: 13/04/2018
   revisao: 22/04/2018
   revisao: 07/05/2018
   revisao: 11/05/2018
   revisao: 03/08/2018 
   revisao: 09/08/2018
   revisao: 12/09/2018
   revisão: 23/09/2018
   revisão: 25/09/2018
   revisão final: 07/12/2018
*/

// --- Bibliotecas Auxiliares ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <SoftwareSerial.h>
// ------------------------------------------------------------------------------------------------------------------------------------

// --- Mapeamento de Portas ---
#define DHTPIN      6
#define pinoPIR     7
#define pinoPotenciometro A2
#define pinRele1    9
#define pinRele2    8                               
#define DHTTYPE DHT22                                  
#define pinoChama   5
#define pinLux      A3
#define pinoBuzzer  A1
#define RX 11
#define TX 12

//----Configuração para conexão internet via ESP01 ----
String AP = "i10 TELECOM - 287";  // Rede
String PASS = "i102018982145";    // Senha
String API = "LAY98YM2W4UHIDYP";  // Chave de escrita ThingSpeak
String HOST = "api.thingspeak.com"; // url da api. Fixa
String PORT = "80";
String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
long tempoGravar = 0;
SoftwareSerial esp8266(RX,TX);

int sensor = 0x50;

boolean flagPIR = false;
boolean flagCHAMA = false;

// --- Configuracao Hardware do LCD ---
LiquidCrystal_I2C lcd(0x27,20,4);
// --- Configuracao Hardware sensor 
DHT dht(DHTPIN, DHTTYPE);
// -------------------------------------------------------------------------------------------------------------------------------------

// --- Configurações Iniciais ---
void setup()
{ 
  //---------------------- CONEXÃO WIFI ----------------------------- 
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  //-----------------------------------------------------------------  

  dht.begin();
  lcd.init();
  lcd.backlight();
  
  pinMode(pinoChama, INPUT);
  pinMode(pinoPIR, INPUT);
  pinMode(pinLux, INPUT);
  pinMode(pinRele1, OUTPUT);
  pinMode(pinRele2, OUTPUT); 
  pinMode(pinoPotenciometro, INPUT);
  pinMode(pinoBuzzer, OUTPUT);
                          
}

void loop()
{  
   int ptUmidade = map(analogRead(pinoPotenciometro), 0, 1023, 1, 100);   //valor recebido pelo potenciometro
   releUmidade(ptUmidade); 
   sensorChama();
   if(flagPIR == true && digitalRead(4) == HIGH){flagPIR = false; 
   digitalWrite(pinoBuzzer,1); delay(50); digitalWrite(pinoBuzzer,0);
   digitalWrite(pinoBuzzer,1); delay(50); digitalWrite(pinoBuzzer,0);}   
   if(digitalRead(3)){flagPIR = true; tone(pinoBuzzer,1400,250);}
   if(flagPIR){movimento();}
   
   temperatura();
} 

void gravar(float x, float y, int z){
   String getData = "GET /update?api_key="+ API +"&field1="+x+"&field2="+y+"&field3="+z;
   sendCommand("AT+CIPMUX=1",5,"OK");
   sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
   sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
   esp8266.println(getData);
    Serial.print("enviado ==> getStr: ");
    Serial.println(getData);
   delay(1500);countTrueCommand++;
   sendCommand("AT+CIPCLOSE=0",5,"OK");  
}

void sensorChama(){
  int valor_d = digitalRead(pinoChama);
  if (valor_d == 0){
    digitalWrite(pinoBuzzer, HIGH);
    delay(7000);
  }else{digitalWrite(pinoBuzzer,LOW);}
  Serial.println(valor_d);
} //end chama

void temperatura(){  
   
   lcd.setCursor(0,0);
   lcd.print(F("T"));
   lcd.setCursor(2,0);
   lcd.print(dht.readTemperature(),1);
   lcd.print(F("C"));

   lcd.setCursor(9,0);
   lcd.print(F("U"));
   lcd.setCursor(11,0);
   lcd.print(dht.readHumidity(),1);
   lcd.print(F("%"));

   lcd.setCursor(0,1);
   lcd.print(F("L"));
   lcd.setCursor(2,1);
   lcd.print(map(analogRead(pinLux), 1023, 0, 0, 100));

   lcd.setCursor(9,1);
   lcd.print(F("R"));
   lcd.setCursor(11,1);
   lcd.print(map(analogRead(pinoPotenciometro), 0, 1023, 0, 100));

  Serial.print("Tempo gravar: ");
  Serial.print(tempoGravar);
  Serial.print("-");
  Serial.println(millis());
   if(millis() > tempoGravar)
  {
    gravar(dht.readTemperature(),dht.readHumidity(),map(analogRead(pinLux), 1023, 0, 0, 100));
    tempoGravar = millis() + 600000; 
  }

     
}

float releUmidade(float a){
   float hu = dht.readHumidity();
      if(hu < a){
          digitalWrite(pinRele1, 1);
        }else{
          digitalWrite(pinRele1, 0);
      }
      return a;
}

void movimento(){
  int movimento = digitalRead(pinoPIR);
   if (movimento == 1) 
   {
      digitalWrite(pinoBuzzer, HIGH);
      delay(5000);
   }else{digitalWrite(pinoBuzzer, LOW);}
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay)){
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false){
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
 }
