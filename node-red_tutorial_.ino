/*
   Tutorial recibiendo datos y controlando cosas
   Este programa recibe datos por medio de
   mqtt enciende ledes y envia datos
   de sensores a un servidor en la nube
   Requerimientos
   PubSubClient libreria
   NodeMCU
   3 Ledes


   Yeffri J. Salazar
   Themicrofcontrol.wordpress.com

*/
/************************************
 **              Librerias         **
 ***********************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
/************************************
 **              constantes         **
 ***********************************/
#define led1 D7
#define led2 D6
#define led3 D5
#define puertoMqtt 1883
/***********************************
 **       objetos y variables      **
 ***********************************/
WiFiClient clienteWifi;//este cliente se encarga de la comunicacion con el wifi
PubSubClient clienteMQTT(clienteWifi);//este utiliza el cliente anterior para hacer poder crear la conexion mqtt
//si pasan por el hackerspace Xibalba pues ya tienen la clave
const char * ssid = "Hackerspace";
const char * claveWifi = "IOT12345";
const char * brokerMqtt = "";// ip del broker sin http ni nada solo los numeros
uint32_t ultimoIntentoReconexion;
uint32_t timerEnvioDatos;
void conectarAlWifi() {

  WiFi.begin(ssid, claveWifi);
  Serial.print("conectando a");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wifi Conectado ");
  Serial.println("direccion IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* mensaje, unsigned int length) {
  String topico = topic;
  /*  String mensaje;
    for(uint8_t i; i <length;i++){
    mensaje+=char(payload[i]);
    }*/
  Serial.print("Mensaje Recibido del topico: ");
  Serial.println(topico);
  Serial.print("mensaje : ");
  Serial.println((char)mensaje[0]);
  if (topico == "/led1") {
    digitalWrite(led1, mensaje[0]);
  }
  else if (topico == "/led2") {
    digitalWrite(led2, mensaje[0]);
  }
  else if (topico == "/led3") {
    digitalWrite(led3, mensaje[0]);
  }
  else {
    Serial.println("error de mensaje");
  }

}

boolean reconexion() {
  Serial.print("Conectando al broker mqtt");
  //intentando conectar al broker
  if (clienteMQTT.connect("ESP8266Client")) {
    Serial.println("Conectado");
    //publicamos que estamos conectados
    clienteMQTT.publish("/conexion", "Conectado");
    //nos suscribimos a los topicos para controlar los ledes
    clienteMQTT.subscribe("/led1");
    clienteMQTT.subscribe("/led2");
    clienteMQTT.subscribe("/led3");
  } else {
    Serial.print("falló, rc=");
    Serial.print(clienteMQTT.state());
  }
  return clienteMQTT.connected();
}

void setup() {
  Serial.begin(115200);
  Serial.println("iniciando programa Holi mami");
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  conectarAlWifi();
  clienteMQTT.setServer(brokerMqtt, puertoMqtt); //le decimos cual es el servidor y el puerto al que se debe conectar
  clienteMQTT.setCallback(callback);//le decimos como se llama la funcion de callback
}

void loop() {
  if (!clienteMQTT.connected()) {
    if (millis() - ultimoIntentoReconexion > 5000) {
      ultimoIntentoReconexion = millis();
      // Attempt to reconnect
      if (reconexion()) {
        ultimoIntentoReconexion = 0;
      }
    }
  } else {
    //cliente conectado 
    if (millis() - timerEnvioDatos > 5000) {
      timerEnvioDatos=millis();
      //a falta de sensores enviamos valores aleatorios
      char msg[4];
      snprintf (msg, 3, "%ld", random(35));
      clienteMQTT.publish("/temperatura", msg);

      snprintf (msg, 4, "%ld", random(100));
      clienteMQTT.publish("/humedad", msg);
    }
    clienteMQTT.loop();

  }
}

