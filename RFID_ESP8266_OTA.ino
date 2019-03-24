//----------------------------------------------------------------
// Arreglo de 1 dimensión para almacenar valor de 4 bytes de la tarjeta
//----------------------------------------------------------------
byte ActualUID[4]; //almacenará código actual de la tarjeta sensada
//----------------------------------------------------------------
// Arreglo de 2 dimensiones para almacenar valor de 4 bytes de cada tarjeta asignada a cada usuario
//----------------------------------------------------------------
byte Usuario[20][4] = { // Reemplace n por la cantidad de usuarios
//----------------------------------------------------------------
// Codigo de Usuarios
//----------------------------------------------------------------
{0xD4,0x7D,0x33,0xC3} , // MasterJP
{0x04,0x9E,0x3A,0xC3} , // Alvaro Delgado
{0x66,0xE4,0x02,0xA4} , // Alvaro Delgado
{0xE4,0x57,0x36,0xC3} , // André Andrade 
{0xE4,0x7F,0x22,0xC3} , // Kevin Ayala
{0x24,0x61,0x39,0xC3} , // Jhayro Chavez
{0xA4,0xE6,0x2E,0xC3} , // Mario Hernández
{0x24,0x2B,0x23,0xC3} , // Carlitos Alburqueque
{0x14,0x62,0x3A,0xC3} , // Victor Cachimbo
{0x44,0x5F,0x26,0xC3} , // Luis Rondón
{0xC4,0x89,0x21,0xC3} , // Carlos Machicado
{0x10,0x51,0x31,0x00} , // Soichi Tamashiro
{0x64,0xBE,0x30,0xC3} , // Ing. Margarita Murillo
{0x14,0x27,0x1E,0xC3} , // Pulpito Herrera
{0xC4,0x0B,0x37,0xC3} , // Samuel
{0x84,0x6D,0x21,0xC3} , // Andrés Sandoval
{0x65,0x00,0x97,0x09} , // Yisus
{0x24,0x24,0x3D,0xC3} , // Owen Mejía
{0x00,0x00,0x00,0x00} , // JImmy Warthon
{0xB4,0x90,0x3C,0xC3} // Usuario de Prueba
};
int cant2 = 21 ; // cant2 = cantidad de usuarios +1
//----------------------------------------------------------------
// Librerías
//----------------------------------------------------------------

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
//Pin 9 para el reset del RC522
#define RST_PIN  D2
//Pin 10 para el SS (SDA) del RC522   
#define SS_PIN  D1
//Creamos el objeto para el RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);
//----------------------------------------------------------------
// Variables
//----------------------------------------------------------------
#ifndef STASSID
#define STASSID "_Put here you wifi ssid_" // Example: "My wifi network"
#define STAPSK  "_Put here your wifi password_" // Example: "*********"
#endif

const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
const char* ssid = STASSID;
const char* password = STAPSK;
//----------------------------------------------------------------
// SETUP
//----------------------------------------------------------------

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }

  MDNS.begin(host);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
  httpServer.handleClient();
  delay(1000);
  // Inicio de Bus Serial  
  Serial.begin(9600);
  // Iniciamos el Bus SPI
  SPI.begin();
  // Iniciamos  el MFRC522
  mfrc522.PCD_Init();
  Serial.println("Control de acceso:");
  // Pin 3 salida hacia el Relay
  pinMode(D4, OUTPUT);
  digitalWrite(D4,HIGH); // dejamos en positivo porque la salida prende en LOW
  
}
void loop(){
//----------------------------------------------------------------
// Refresca el servidor Wifi para la actualización de firmware
//----------------------------------------------------------------
  httpServer.handleClient();
//----------------------------------------------------------------
// Comprueba el estado actual del lector RFID
//----------------------------------------------------------------
  // Revisamos si hay nuevas tarjetas presentes
  if ( mfrc522.PICC_IsNewCardPresent()){
    //Seleccionamos una tarjeta
    if ( mfrc522.PICC_ReadCardSerial()) {
      // Enviamos serialemente su UID
      //Serial.print(F("Card UID:"));
      for (byte i = 0; i < mfrc522.uid.size; i++) {
                          // Descomentar para ver el código de la tarjeta RFID
                          //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                          //Serial.print(mfrc522.uid.uidByte[i], HEX);   
                          ActualUID[i]=mfrc522.uid.uidByte[i];          
      }
//----------------------------------------------------------------
// Función para comparar tarjeta actual con la base de tarjetas en el programa
//----------------------------------------------------------------
          for (int ini = 0;ini<cant2;ini++){
            if (compareArray(ActualUID,Usuario,ini)){
            Serial.print("Acceso Autorizado\n");
            digitalWrite(3,LOW);    // Se deja en negativo porque la salida prende en LOW de lo contrario commentar la linea si prende en HIGH
            //digitalWrite(3,HIGH); // Se deja en postivo porque la salida prende en HIGH de lo contrario commentar la linea si prende en LOW
            delay(500);             // Tiempo de retraso para que funcione el relay
            digitalWrite(3,HIGH);   // Se deja en positivo porque la salida prende en LOW de lo contrario commentar la linea si prende en HIGH
            //digitalWrite(3,LOW);  // Se deja en negativo porque la salida prende en HIGH de lo contrario commentar la linea si prende en LOW
            }
            else{
            Serial.print("No Autorizado\n");
                       
            }
            
          }
        
        }
  }
}  
//----------------------------------------------------------------
// Función para comparar dos vectores el primero de 1 dimensión y el segundo de 2 dimensiones con un selector de cual fila comparar
//----------------------------------------------------------------
boolean compareArray(byte array1[],byte array2[][4],int i)
{
  int b=i;
  if(array1[0] != array2[b][0])return(false);
  if(array1[1] != array2[b][1])return(false);
  if(array1[2] != array2[b][2])return(false);
  if(array1[3] != array2[b][3])return(false);
  return(true);
}
