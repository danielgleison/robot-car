#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

WiFiClient client;
WiFiServer server(80);
const char* ssid = "RobotCar";
const char* password = "1234";


//DEFINIÇÃO DE IP FIXO PARA O NODEMCU
IPAddress ip(192,168,0,200); 
IPAddress gateway(192,168,0,1); 
IPAddress subnet(255,255,255,0); 

String  command =""; // Command received from Android device

#include <Ultrasonic.h> 

//Configuração
int vel_min = 500;
int vel_normal = 750;
int vel_max = 1023;
int tempo_desvio = 2000; //delay em ms
int speedCar = 800;  // velocidade do carro (400 - 1023)
int speed_Coeff = 2;
int dist_min = 20; // distância mínima para desvio (cm)
int ModoRemAuto = 0; // 0 - modo remoto / 1 - modo automático
int luz_min = 50; // 0 - modo remoto / 1 - modo automático


/*
 * Pinos
 * Não utilizar GPIOs: 6, 7, 8, 9 (SD2) e 11
 * Utilizar o RX (GPIO 3) como saída digital
 * Não utilizar o TX (GPIO 1) como saída digital
 * D0 não tem PWM
 * 
*/
const int rightMotor2 = D3;    // GPIO 13
const int rightMotor1 = D4;    // GPIO 15
const int leftMotor2 = D7;    // GPIO 0
const int leftMotor1 = D8;    // GPIO 2
const int eneLeftMotor = D5;  // GPIO 12
const int eneRightMotor = D6; // GPIO 14

const int PinLed = 10; //GPIO 10 - SD3
const int PinBuzzer = D0; //GPIO 16
#define TRIGGER_PIN D1       
#define ECHO_PIN    D2

//Sensor Ultrassonico
Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
int dist_num_leitura = 1; 
float dist_sinal;   
float dist_soma = 0;  
float dist_cm = 0;


//LDR
const int Sensor_LDR = A0; //entrada analógica do LDR
int LDR_num_leitura = 1; //Define o numero de medicoes
long LDR_sinal;   //Armazena o valor lido do Sensor de Som
long LDR_soma = 0; //Armazena o valor total das n medicoes
long LDR_valor = 0;
int IntensidadeLuz = 0;


/* connecting WiFi */
void CONNECTWIFI()
{
   Serial.println("Connecting to WIFI");

  //Modo Station (STA)
   // WiFi.mode(WIFI_STA);
   // WiFi.begin(ssid, password);
   // WiFi.config(ip, gateway, subnet);


  //Modo Access Point (AP) - IP padrão do NodeMCU = 192.168.4.1
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  //softAPConfig (local_ip, gateway, sub-rede);

  //Modo STA_AP
   //WiFi.mode(WIFI_STA_AP);
   //WiFi.config(local_ip, gateway, subnet, dns1, dns2);
   //WiFi.softAP(ssid, password);
     
 
  //while ((!(WiFi.status() == WL_CONNECTED)))
  //{
  //  delay(300);
  //  Serial.print("..");
  //}
  
  Serial.println("");
  //Serial.println("WiFi connected");
  Serial.println("NodeMCU Local IP is : ");
  //Serial.print((WiFi.localIP())); //Endereço STA
  Serial.print(WiFi.softAPIP()); //Endereço AP
  Serial.println("");
}

void setup()
{
  Serial.begin(115200);

  pinMode (eneLeftMotor, OUTPUT); 
  pinMode (eneRightMotor, OUTPUT); 
  pinMode (leftMotor1, OUTPUT); 
  pinMode (leftMotor2, OUTPUT);  
  pinMode (rightMotor1, OUTPUT);  
  pinMode (rightMotor2, OUTPUT);
  pinMode (PinLed, OUTPUT);
  pinMode (PinBuzzer, OUTPUT);  

  CONNECTWIFI();

  server.begin(); 

  ModoRemAuto = 0; // modo remoto
  SetLed(0);
 }


void SetLed (int State)
{
 digitalWrite(PinLed, State);  
}


void PlayBuzzer (int frequencia, int duracao, int tempo)
{
  tone(PinBuzzer, frequencia , duracao);
  delay (tempo);
  noTone(PinBuzzer);
}


void DISTANCIA()
{
  float cmMsec;
  long microsec = ultrasonic.timing();  //Le os dados do sensor, com o tempo de retorno do sinal
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);  //Calcula a distancia em centimetros
  
  //Efetua n leituras do sinal 
  dist_soma = 0; 
  for (int i=0; i < dist_num_leitura ; i++)  
  {  
    dist_sinal = cmMsec;  
    dist_soma = dist_soma + dist_sinal;  
  }  
  dist_cm = dist_soma / dist_num_leitura;

  Serial.print("Distancia = ");
  Serial.print(dist_cm);
  Serial.println("cm");
  
 }


void LUMINOSIDADE()
  {

    LDR_soma = 0; //Zera a variável LDR
    for (int i = 0; i <= LDR_num_leitura; i++) //Faz n vezes a leitura do sensor
    {
      LDR_sinal = analogRead(Sensor_LDR);//Faz a leitura do sensor, em um valor que pode variar de 0 a 1024
      LDR_valor = map(LDR_sinal, 0, 1023, 0, 100); //Converte o valor para uma escala de 0 a 100
      LDR_soma = LDR_soma + LDR_valor;
    }
   IntensidadeLuz = LDR_soma / LDR_num_leitura;

    Serial.print("LDR = ");
    Serial.print(IntensidadeLuz);
    Serial.println("%");
  }


/* command motor forward */
void forwardMotor(void)   
{
  analogWrite(eneLeftMotor, speedCar);
  //delay(30);
  analogWrite(eneRightMotor, speedCar);
  //delay(30);
     
  digitalWrite(leftMotor1,LOW);
  digitalWrite(leftMotor2,HIGH);
  digitalWrite(rightMotor1,LOW);
  digitalWrite(rightMotor2,HIGH);

}

/* command motor backward */
void reverseMotor(void)   
{
  analogWrite(eneLeftMotor, speedCar);
  //delay(30);
  analogWrite(eneRightMotor, speedCar);
  //delay(30);
  
  digitalWrite(leftMotor1,HIGH);
  digitalWrite(leftMotor2,LOW);
  digitalWrite(rightMotor1,HIGH);
  digitalWrite(rightMotor2,LOW);
}

/* command motor turn left */
void leftMotor(void)   
{
  analogWrite(eneLeftMotor, speedCar);
  //delay(30); 
  analogWrite(eneRightMotor, speedCar);
  //delay(30); 
  
  digitalWrite(leftMotor1,LOW);
  digitalWrite(leftMotor2,LOW);
  digitalWrite(rightMotor1,LOW);
  digitalWrite(rightMotor2,HIGH);
}

/* command motor turn right */
void rightMotor(void)   
{
  analogWrite(eneLeftMotor, speedCar);
  //delay(30);
  analogWrite(eneRightMotor, speedCar);
  //delay(30);
  
  digitalWrite(leftMotor1,LOW);
  digitalWrite(leftMotor2,HIGH);
  digitalWrite(rightMotor1,LOW);
  digitalWrite(rightMotor2,LOW);
}

/* command motor stop */
void stopMotor(void)   
{
  analogWrite(eneLeftMotor, 0);
  //delay(30);
  analogWrite(eneRightMotor, 0);
  //delay(30);

  digitalWrite(leftMotor1,LOW);
  digitalWrite(leftMotor2,LOW);
  digitalWrite(rightMotor1,LOW);
  digitalWrite(rightMotor2,LOW);
}


void MODOAUTONOMO()
{
 
  if(dist_cm < dist_min)                
   {
    stopMotor(); Serial.println("Parado");
    speedCar = vel_min;
    delay(tempo_desvio);
    reverseMotor(); Serial.println("Reverso");
    delay(tempo_desvio);
    leftMotor(); Serial.println("Esquerda");
    delay(tempo_desvio);
    stopMotor(); Serial.println("Parado");
    delay(tempo_desvio);
   }
   
  else
   {
   speedCar = vel_normal;
   forwardMotor(); Serial.println("Frente");
   }


   if(IntensidadeLuz < luz_min) 
   {
    SetLed(1);
    Serial.println("Farol ligado");
   }
   else 
   {
    SetLed(0);
    Serial.println("Farol desligado");
   }
   
}

/* check command received from Android Device */
String checkClient (void)
{
  while(!client.available()) delay(1); 
  String request = client.readStringUntil('\r');
  request.remove(0, 5);
  request.remove(request.length()-9,9);
  return request;
}


/* send command echo back to android device */
void sendBackEcho(String echo)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println(echo);
  client.println("</html>");
  client.stop();

  Serial.println(echo); 
  
  delay(1);
}


void loop() 
{

   if (ModoRemAuto == 1) 
   {
    MODOAUTONOMO();
    Serial.println("Modo Automático");
   }
   else
   {
    Serial.println("Modo Remoto");
   }

   
   DISTANCIA();
   LUMINOSIDADE();

  client = server.available();
    if (!client) return; 
    command = checkClient ();

  if ((command == "forward" || command == "frente" || command == "para frente" || command == "seguir" || command == "vai") && (dist_cm > dist_min)) forwardMotor();
  else if (command == "reverse" || command == "reverso" || command == "para trás" || command == "voltar" || command == "volta" || command == "retorno") reverseMotor();
  else if (command == "left"    || command == "esquerda" || command == "lado esquerdo") leftMotor();
  else if (command == "right"   || command == "direita"  || command == "lado direito") rightMotor();
  else if (command == "stop"    || command == "pare" || command == "parar" || command == "parado") stopMotor();
  else if (command == "lento") speedCar = vel_min;
  else if (command == "normal") speedCar = vel_normal;
  else if (command == "turbo") speedCar = vel_max; 
  
  else if (command == "farolon") SetLed(1);
  else if (command == "faroloff") SetLed(0);
  else if (command == "alertaon") SetLed(1);
  else if (command == "alertaoff") SetLed(0);
  else if (command == "buzzeron") PlayBuzzer (2000, 200, 2000); // (frequência [Hz], duração [ms], delay [ms])
  else if (command == "buzzeroff") noTone(PinBuzzer);
  else if (command == "automatico") ModoRemAuto = 1;
  else if (command == "remoto") ModoRemAuto = 0;

     
    sendBackEcho(command); // send command echo back to android device
    command = "";
    


  delay (10);

}
