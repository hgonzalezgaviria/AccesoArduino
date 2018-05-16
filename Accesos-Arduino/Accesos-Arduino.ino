/* ----------------------------------------------------------------------
   Pines seleccionados para el ESP8266 NodeMCU Amica V2
   Signal     Pin              Pin               Pin              Pin			Pin 		Pin
            ESP8266          LCD I2C         MFRC522 board     	Buzzer		  led red	 led verde
   -------------------------------------------------------------------------------------------------
   RST	      D4(GPIO02)                         RST
   SPI SS     D8(GPIO15)                         SDA
   SPI MOSI   D7(GPIO13)                         MOSI
   SPI MISO   D6(GPIO12)                         MISO
   SPI SCK    D5(GPIO14)                         SCK   
   SDA        D2(GPIO04)      SDA
   SCL        D1(GPIO05)      SCL
   SD3        SD3(GPIO10)      									 SD3       
   D3         D3(GPIO0)      									 				 D3       
   D0         D0(GPIO16)			      									 				 D0
   5V         VIN             VIN                                
   3.3V       3V3                                3.3V
   GND        GND             GND                GND             GND			GND			GND
*/

#include <time.h> // Libreria para la Hora
#include <Wire.h> // Libreria interna para escribir en el Display
#include <LiquidCrystal_I2C.h> //Libreria para manejo del modelo i2C con Display 16x2

#include <SPI.h> //Libreria para el menejo de PINES
#include <MFRC522.h> // Libreria para tarjeta RFID RC522
#include <ESP8266WiFi.h>  //Libreria propia para el ESP8266, manejo del wifi
#include <ESP8266HTTPClient.h> //Libreria propia para el ESP8266
#include <ArduinoJson.h> //Libreria para peticiones de json http


const char* ssid="HUAWEI-E156";
const char* password = "nta9hdde";
//const char* ssid="Troyawifi";
//const char* password = "pasionporlavida**";

//Tiempo
int timezone = 7;
char ntp_server1[20] = "3.th.pool.ntp.org";
char ntp_server2[20] = "1.asia.pool.ntp.org";
char ntp_server3[20] = "0.asia.pool.ntp.org";
int dst = 0;
bool displayTime = 0;

//PINES PARA RFID-RC522
#define SS_PINESP 15    // (D8) pin que se conecta con el pin SDA del RFID-RC522
#define RST_PINESP 2   // (D4) pin que se conecta con el pin RST del RFID-RC522

//Leds y buzzer
int ledRedPin = 0; // (D3) GPIO0
int ledGreenPin = 16; // (D1) GPIO16
int buzzerPin = 10; // (SD3) GPIO10


LiquidCrystal_I2C lcd(0x3F, 16, 2);  //Se inicializa objeto lcd
MFRC522 mfrc522(SS_PINESP, RST_PINESP);   // Crea instacia de MFRC522.

// Metodo de configuración
void setup() {
    Serial.println("Abrir Leds");
  pinMode(ledRedPin,OUTPUT);
  pinMode(ledGreenPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
   Serial.begin(115200);// Inicializa la comunicación serial del puerto local.

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("................");
  
  Serial.println();  
  SPI.begin();      // Inicia el bus SPI.
  mfrc522.PCD_Init();   // Inicializa MFRC522
  // Se estabalecen los pines S2 y S3 como salida
  //pinMode(ledGreenPin, OUTPUT); 
  //pinMode(ledRedPin, OUTPUT);
  // Se realiza la conexión a la red WiFi
  Serial.println("Llamada al metodo de wifi");
  ConectarWiFi();
  //Tiempo 
    delay(1000);
  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  delay(1000);
    lcd.clear();
  DisplayWAiT_CARD();
}

// Este metodo es el que se ejecuta siempre
void loop() {
//Tiempo-- Hora en LCD
   time_t now = time(nullptr);
  struct tm* newtime = localtime(&now);

  if (displayTime == 0) {

    if (!time(nullptr)) {
      lcd.setCursor(0, 0);
      lcd.print("    WELCOME     ");
    }
    else
    {
      lcd.setCursor(4, 0);
      if ((newtime->tm_hour) < 10)lcd.print("0");
      lcd.print(newtime->tm_hour);
      lcd.print(":");

      lcd.setCursor(7, 0);
      if ((newtime->tm_min) < 10)lcd.print("0");
      lcd.print(newtime->tm_min);
      lcd.print(":");

      lcd.setCursor(10, 0);
      if ((newtime->tm_sec) < 10)lcd.print("0");
      lcd.print(newtime->tm_sec);
    }
  }
  //Tiempo-- Hora en LCD

  //Codigo que verifica que siempre exista la conexión a Internet, para asegurar la estabilidad del servicio.
  if (WiFi.status() == WL_CONNECTED) {
    // Buscar tag rfid nuevo
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    // Selecciona uno de los tags
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    // Se procede a realizar la lectura del RFID
    readRFID();
  } else {
    Serial.println("Se perdió la conexión");
    digitalWrite(ledGreenPin, HIGH);
    delay(200);
    digitalWrite(ledGreenPin, LOW);
    delay(200);
    
    digitalWrite(ledRedPin, HIGH);
    delay(200);
    digitalWrite(ledRedPin, LOW);
    delay(200);
  }
}

// Método para conectarse a una red WiFi
void ConectarWiFi() {
  Serial.println("Conectandose a la red WiFi");
    Serial.println( ssid );
    Serial.println();
  WiFi.begin(ssid,password);
   Serial.print("Connecting..");
          
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    digitalWrite(ledGreenPin, HIGH);
    digitalWrite(ledRedPin, HIGH);
    delay(250);
    digitalWrite(ledGreenPin, LOW);
    digitalWrite(ledRedPin, LOW);
    delay(250);
  }
     
  Serial.print("Conexión exitosa - IP: ");
  Serial.print(WiFi.localIP());
  Serial.println();
}

// Metodo que obtiene el UID de el tag que se esté leyendo
void readRFID() {
  
  // Mostrar el UID del tag
  Serial.println();
  Serial.print("UID tag: ");
  Serial.println();
  String content = "";
  byte letter;

  // Se lee letra por letra el UID qe devuelve el tag  
  for (byte i = 0; i < mfrc522.uid.size; i++) {
   Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
   Serial.print(mfrc522.uid.uidByte[i], HEX);
   content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
   content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  // Se convierte todo a mayúscula
  content.toUpperCase();
  // Se quitan los espacios ej: XXBB67RR
  content.replace(" ", "");
  Serial.println();
  Serial.println(".");
  Serial.println(content.substring(1));
  
  
  String urlServicio = "http://192.168.8.100:8081/Acceso/public/verificarAcceso?tagid=" + content.substring(1);
  // Se hace la petición al servidor y se le envían todos los parámetros
  int resultado = requestServer(urlServicio);
  
  if (resultado == 1) {
    Serial.println("Acceso Autorizado");
    Serial.println("Entra...");
  

        digitalWrite(ledGreenPin, HIGH);
    delay(1200);
    digitalWrite(ledGreenPin, LOW);

      analogWrite(buzzerPin,20);
    delay(500);
    analogWrite(buzzerPin,0);
    Serial.println();

        lcd.clear(), lcd.setCursor(0, 0), lcd.print(" Bienvenido!"), lcd.setCursor(0, 1), lcd.print("                "), lcd.setCursor(0, 1), lcd.print("ID "); 
    lcd.print(content.substring(1));
    delay(2600);
    DisplayWAiT_CARD();
    
  } else if ( (resultado == 2)){
    Serial.println("Acceso Autorizado");
    Serial.println("Sale...");
  

        digitalWrite(ledGreenPin, HIGH);
    delay(1200);
    digitalWrite(ledGreenPin, LOW);

      analogWrite(buzzerPin,20);
    delay(500);
    analogWrite(buzzerPin,0);
    Serial.println();

        lcd.clear(), lcd.setCursor(0, 0), lcd.print(" Adios!"), lcd.setCursor(0, 1), lcd.print("                "), lcd.setCursor(0, 1), lcd.print("ID "); 
    lcd.print(content.substring(1));
    delay(2600);
    DisplayWAiT_CARD();
    
  } else if ( (resultado == 0)) {
    Serial.println("Acceso denegado");
    digitalWrite(ledRedPin, HIGH);
    delay(1200);
    digitalWrite(ledRedPin, LOW);
         analogWrite(buzzerPin,30);
    delay(1000);
    analogWrite(buzzerPin,0);

    lcd.setCursor(0, 0), lcd.print(" Card not Found"), lcd.setCursor(0, 1), lcd.print("                "), lcd.setCursor(0, 1), lcd.print("ID "); 
    lcd.print(content.substring(1));
    delay(2000);
    DisplayWAiT_CARD();
 
  }
  else {
    Serial.println("Problema de conexion al Servidor");
  }
}

// Se encarga de realizar la petición WEB al servidor
int requestServer(String url) {
  HTTPClient http;
  http.begin(url);
  
  // Si la petición es exitosa
  if (http.GET() == 200) {
    const size_t bufferSize = JSON_OBJECT_SIZE(1) + 38;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& json = jsonBuffer.parseObject(http.getString());
    Serial.println(http.getString());
    String res = json["success"];
    return res.toInt();
  } else {
    Serial.println(http.getString());
  }
  return 404;
}

// -------------------------------------------

void DisplayWAiT_CARD()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Please Tag Card");
}

// -------------------------------------------
