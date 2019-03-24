//----------------------------------------------------------------
// Arreglo de 1 dimensión para almacenar valor de 4 bytes de la tarjeta
//----------------------------------------------------------------
byte ActualUID[4]; //almacenará código actual de la tarjeta sensada
//----------------------------------------------------------------
// Arreglo de 2 dimensiones para almacenar valor de 4 bytes de cada tarjeta asignada a cada usuario
//----------------------------------------------------------------
byte Usuario[20][4] = { // Reemplace n por la cantidad de usuarios
//----------------------------------------------------------------
// CÓDIGOS DE USUARIOS
//----------------------------------------------------------------
{0x00, 0x01, 0x03, 0x04}, // Usuario 0
{0x05, 0x06, 0x07, 0x08}, // Usuario 1
//                    .
//                    .
//                    .
{0xFF, 0xFF, 0xFF, 0xFF}  // Usuario n
};
int cant2 =/* n+1 */; // cant2 = cantidad de usuarios +1
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
