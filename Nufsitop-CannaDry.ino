/* * * * * TFT Display  * * * * */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_MOSI 19          // Pinbelegungen von https://github.com/Xinyuan-LilyGO/TTGO-T-Display
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_BL 4

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

/* * * * * WIFI  * * * * */
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
WiFiMulti wifiMulti;
String ssid = "";
char* password = "";

/* * * * * SHELLY  * * * * */
String shelly_sn = "", shelly_id;
int shelly_RSSI;
int shelly_on = 3;
unsigned int shellyFound = 0, trigger_radon, trigger_iaq;
unsigned int shelly_pow, shelly_meter, shelly_return;

/* * * * * BME280 * * * * */
#include <BME280I2C.h>
#include <Wire.h>
BME280I2C bme;         // Default : forced mode, standby time = 1000 ms
float BMEtemp, BMEhum; // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
int tempLow = 15, tempHigh = 18;

// macadr -> sn
unsigned long mac_adr;
String sn = "";

unsigned long previousMillisBME280 = 0;  // Variable zum Speichern des letzten Zeitpunkts für printBME280
unsigned long previousMillisWifi = 0;    // Variable zum Speichern des letzten Zeitpunkts für sendDataWifi
unsigned long currentMillis = millis();  // Variable zum Speichern des aktuellen Zeitpunkts
const long intervalBME280 = 60000;       // Intervall von einer Minute (60000 Millisekunden)
const long intervalWifi = 770000;        // Intervall von 13 Minuten (780000 Millisekunden)

int greenhemp = 180;
int colorhemp = 100;
int displayTime = 2000;

#include "hempb.h"
#include "hemp.h"

void setup(void) {
  delay(3000);
  pinMode(TFT_BL, OUTPUT);      // TTGO T-Display Backlight Pin 4 aktivieren
  delay(1000);  
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Nufsitop V0.2"));
  delay(1000);
  sn = generateMacSerial();
  Serial.println("Flash:" + showDateGerman() + "SN: " + sn);
  delay(1000);

  tftInit();
  Serial.println(F("showLogo"));
  showLogo();

  initBME280(); 
  getBME280();
  tft4Lines("NUF SI TOP", "Temp.:   " + String(BMEtemp,0) + "C", "Feuchte: " + String(BMEhum,0) + "%", "-> Steckdose?", 2000);

  wifi_scan(); 

  shelly_on = shelly_wifi(0);
  if (shelly_on == 0) {
    tft4Lines("NUF SI TOP", " ", "Verbindung", "Steckdose OK", 2000);
  } else {
    tft4Lines("NUF SI TOP", "Verbindung", "Steckdose", "gescheitert!", 2000);
  }

  showLosGehts();
  backlightOff();
}

void loop() { currentMillis = millis();
// ALLE 13 MINUTEN
  if (currentMillis - previousMillisWifi >= intervalWifi) {     // Überprüft, ob 13 Minuten vergangen sind
    previousMillisWifi = currentMillis;                         // Speichert den aktuellen Zeitpunkt
    wifiMulti.addAP("Studio2", "Wolf1212");
    //wifiMulti.addAP("ALARM4U", "Wolf1212");
    delay(100);
    sendDataWifi();
    delay(100);
    WiFi.disconnect();  
    tftInit();
    tft4Lines("NUF SI TOP", "Datentransfer", "Temp.:   " + String(BMEtemp,0) + "C", "Feuchte: " + String(BMEhum,0) + "%", 5000);
    backlightOff();          
  }

// JEDE MINUTE
  if (currentMillis - previousMillisBME280 >= intervalBME280) { // Überprüft, ob eine Minute vergangen ist
    previousMillisBME280 = currentMillis;                       // Speichert den aktuellen Zeitpunkt
    printBME280();
    tftInit();
    if (BMEtemp > tempHigh && shelly_on == 0) {
      shelly_wifi(1);
      Serial.println(String(BMEtemp,1) + ">" + String(tempHigh) + " - Shelly AN");
      tft4Lines("NUF SI TOP", "Temp.:   " + String(BMEtemp,0) + "C", "Feuchte: " + String(BMEhum,0) + "%", "Steckdose AN", 5000);
    } else if (BMEtemp < tempLow && shelly_on == 1) {
      shelly_wifi(0);
      Serial.println(String(BMEtemp,1) + "<" + String(tempLow) + " - Shelly Aus");
      tft4Lines("NUF SI TOP", "Temp.:   " + String(BMEtemp,0) + "C", "Feuchte: " + String(BMEhum,0) + "%", "Steckdose AUS", 5000);
    } else {
      tft4Lines("NUF SI TOP", "Temp.:   " + String(BMEtemp,0) + "C", "Feuchte: " + String(BMEhum,0) + "%", "Steckdose: " + String(shelly_on), 5000);
    }
    backlightOff();
  }
}



void sendDataWifi() {                           
  if ((wifiMulti.run() == WL_CONNECTED)) {      // Überprüft, ob das Gerät mit einem WiFi-Netzwerk verbunden ist
    HTTPClient http;                            // Erstellt ein HTTPClient-Objekt
    Serial.print("[HTTP] begin...\n");
    // URL für die HTTP-Anfrage            Sensor;GSMTRY;BQM;STATUS;TMP;HUM;HPA;IAQ;STA_IAQ;POW_1;METER;$EPOCH
    http.begin("http://con.radocon.de/r.php?a="+sn+";1;0;1;"+String(BMEtemp*10, 0)+";"+String(BMEhum*10, 0)+";990;0;0;0;0;1");  // HTTP
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();                  // Führt die HTTP GET-Anfrage aus

    if (httpCode > 0) {                         // Überprüft, ob die Anfrage erfolgreich war
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {           // Überprüft, ob der HTTP-Statuscode OK (200) ist
        String payload = http.getString();      // Ruft die Antwort als String ab
        Serial.println(payload);                // Gibt die Antwort auf dem Serial-Monitor aus
      }
    } else {                                    // Gibt eine Fehlermeldung aus, falls Fehlgeschlag
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();                                 // Beendet die HTTP-Verbindung
  }
}






