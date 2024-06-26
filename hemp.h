
void dly() {
  delay(1000);
}


void tft4Lines(String text1, String text2, String text3, String text4, int displayTime) {
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp));
  tft.setTextSize(3);             // Textgröße einstellen
  tft.setCursor(3, 8);         // Position für den Text setzen
  tft.println(text1);          // Erste Zeile Text
  tft.setCursor(3, 39);         // Position für die zweite Zeile Text setzen
  tft.println(text2);
  tft.setCursor(3, 70);         // Position für die dritte Zeile Text setzen
  tft.println(text3);
  tft.setCursor(3, 101);         // Position für die dritte Zeile Text setzen
  tft.println(text4);
  delay(displayTime);
} 

void disconnectWifi() {
    if (WiFi.status() == WL_CONNECTED) {    
        WiFi.disconnect(true);
        delay(100);
    }
 }

void wifi_scan()
{
    WiFi.mode(WIFI_STA);
    delay(100);
    int routerRSSI;
    String router, shelly, t1 = "", t2 = "";
    int16_t n = WiFi.scanNetworks();
    shellyFound = 0;
    if (n < 1) {
      Serial.println("keine Netzwerke");
      t1 = "keine Netzwerke";
    } else {
         Serial.printf("%d Netzwerke\n", n);
         t1 = String(n) + " Netzwerke";
        for (int i = 0; i < n; ++i) {
            router = WiFi.SSID(i).c_str();  
            routerRSSI = 101 + WiFi.RSSI(i);     
            //Serial.println(router + " " + String(routerRSSI));          
            if (router.length() == 28){
              if (router.substring(0,16) == "ShellyPlusPlugS-"){
                Serial.print("Shelly found! - SN: ");
                shelly_RSSI = routerRSSI;                  
                shelly_sn = router.substring(16,28);;
                Serial.println(shelly_sn + " - RSSI: " + String(shelly_RSSI));         
                shellyFound = 1;       
                t2 = "1 Steckdose";
              }
            }
        }
    }
    // WiFi.mode(WIFI_OFF);
    tft4Lines("NUF SI TOP", " ", t1, t2, 2000);
    delay(100);
}


int shelly_wifi(int shelly_task) {              // 0 für Relay aus 1 für Relay an
    long timeout = millis();                    // Speichert den aktuellen Zeitpunkt in Millisekunden
    unsigned int shelly_return = 9999;          // Initialwert für den Rückgabewert
    ssid = "ShellyPlusPlugS-" + shelly_sn;      // Setzt die SSID für das Shelly-Gerät
    WiFi.mode(WIFI_STA);                        // Setzt den WiFi-Modus auf Station
    WiFi.begin(ssid.c_str(), password);         // Verbindet mit dem WiFi-Netzwerk
    while (WiFi.status() != WL_CONNECTED) {     // Wartet, bis die WiFi-Verbindung hergestellt ist
      if ((millis() - timeout) > 15000) {       // Bricht nach 15 Sekunden ab
        break;
      }
    }
    shelly_id = "1 ID:" + WiFi.SSID() + "S:" + (WiFi.RSSI() * -1); // Setzt die shelly_id mit SSID und Signalstärke
    delay(100);                                 // Wartet 100 Millisekunden
    const uint16_t port = 80;                   // Setzt den Zielport
    const char * host = "192.168.33.1";         // Setzt die Ziel-IP-Adresse
    WiFiClient client;                          // Erstellt einen WiFiClient
    if (!client.connect(host, port)) {          // Verbindet zum Host, falls nicht erfolgreich
        shelly_return = 8888;                   // Setzt den Rückgabewert auf 8888
        return shelly_return;                   // Gibt den Rückgabewert zurück
    }

    if (shelly_task == 0) {                     // Schaltet das Relais aus         
        client.print("GET /relay/0?turn=off HTTP/1.1\n\n");
        shelly_return = 0;
        shelly_on = 0;
    } else {                                    // Schaltet das Relais ein
        client.print("GET /relay/0?turn=on HTTP/1.1\n\n");    
        shelly_return = 1;
        shelly_on = 1;
    }
    int maxloops = 0;                           // Initialisiert eine Schleifenvariable
    while (!client.available() && maxloops < 1000) {          // Wartet, bis Daten verfügbar sind
        maxloops++;
        delay(1);                               // Wartet 1 Millisekunde
    }
    String watt;  
    if (client.available() > 0) {                 // Wenn Daten verfügbar sind
        String line = client.readStringUntil('{'); // Liest bis zum ersten '{'
        line = client.readStringUntil(':');       // Liest bis zum ersten ':'
        line = client.readStringUntil(',');       // Liest bis zum ersten ','
        shelly_id = line + " " + shelly_id;       // Aktualisiert die shelly_id
        watt = line;                              // Setzt watt auf den gelesenen Wert
    } else {
        Serial.println("client.available() timed out "); // Meldet Timeout
    }
    client.stop();                                // Beendet die Verbindung zum Server
    WiFi.disconnect();                            // Trennt die WiFi-Verbindung
    shelly_id = shelly_id + " T:" + (millis() - timeout) + "ms"; // Aktualisiert shelly_id mit der verstrichenen Zeit
    if (shelly_task > 1) {
        shelly_return = watt.toInt();             // Setzt den Rückgabewert auf den gelesenen watt-Wert
    } 
    return shelly_return;                         // Gibt den Rückgabewert zurück
}

void backlightOn() {
  digitalWrite(TFT_BL, HIGH);   // T-Display Backlight einschalten
}

void backlightOff() {
  digitalWrite(TFT_BL, LOW);    // Schaltet das Backlight aus
}

void tftInit() {
  digitalWrite(TFT_BL, HIGH);   // T-Display Backlight einschalten
  tft.init(135, 240);           // ST7789 initialisieren 240x135
  tft.setRotation(3);           // Optional: Display-Rotation einstellen
}


void showLosGehts() {
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
  tft.drawRGBBitmap(10, 0, hemp, 128, 128); // Bild anzeigen

  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setTextSize(3);             // Textgröße einstellen
  tft.setCursor(165, 25);         // Position für den Text setzen
  tft.println(F("OK"));          // Erste Zeile Text
  tft.setCursor(158, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("LOS"));           // Zweite Zeile Text
  tft.setCursor(145, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("GEHTS"));          // Dritte Zeile Text
  delay(2000);
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
}


void showLogo() {
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
  tft.drawRGBBitmap(10, 0, hemp, 128, 128); // Bild anzeigen

  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setTextSize(3);             // Textgröße einstellen
  tft.setCursor(160, 25);         // Position für den Text setzen
  tft.println(F("NUF"));          // Erste Zeile Text
  tft.setCursor(165, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("SI"));           // Zweite Zeile Text
  tft.setCursor(160, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("TOP"));          // Dritte Zeile Text

  delay(1500);
  tft.setTextColor(ST77XX_BLACK); // Textfarbe auf Schwarz setzen
  tft.setCursor(165, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("SI"));           // Zweite Zeile Text
  delay(100);
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setCursor(165, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("IS"));           // Zweite Zeile Text
  delay(300);
  tft.setTextColor(ST77XX_BLACK); // Textfarbe auf Schwarz setzen
  tft.setCursor(160, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("TOP"));          // Dritte Zeile Text
  delay(100);
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setCursor(160, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("FUN"));          // Dritte Zeile Text
  delay(300);
  tft.setTextColor(ST77XX_BLACK); // Textfarbe auf Schwarz setzen
  tft.setCursor(160, 25);         // Position für den Text setzen
  tft.println(F("NUF"));          // Erste Zeile Text
  delay(100);
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setCursor(160, 25);         // Position für den Text setzen
  tft.println(F("POT"));          // Erste Zeile Text
  delay(2000);
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
}



void BME280Data()
{
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(pres, temp, hum, tempUnit, presUnit);
  BMEtemp = temp;
  BMEhum = hum;
}

void getBME280()
{
  BME280Data();
  delay(100);  
  BME280Data();
}

void printBME280()
{
  getBME280();
  Serial.println("Tmp: " + String(BMEtemp,0) + "C -  Hum:" + String(BMEhum,0) + "%");
}

void initBME280() {
  Wire.begin();
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
  delay(100);
  BME280Data();
  delay(100);  
  BME280Data();
  delay(100);  
}

String convertToArbitraryBase(unsigned long value, int base) {
  static char baseChars[] = "ZWAFBCDEFGHIJKLMNZPFRSTUV71234WXYZABCDEF7123456789GHIJKLM6789NZPHRSTUVWXYZPB";
  String result = "";
  do {
    result = String(baseChars[value % base]) + result; // Add on the left
    value /= base;
  } while (value != 0);
  return result; 
} // Spezielle convertToArbitraryBase Funktion

String generateMacSerial() {
  WiFi.begin();  // Start WiFi to get MAC address
  String macStr = WiFi.macAddress();  // Buffer for the MAC address
  WiFi.disconnect(true); // Trennt die Verbindung und schaltet das WiFi aus
  WiFi.mode(WIFI_OFF);   // Setzt das WiFi-Modul in den OFF-Modus
  uint8_t baseMac[6];  // Parse MAC address
  sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
         &baseMac[0], &baseMac[1], &baseMac[2], &baseMac[3], &baseMac[4], &baseMac[5]);
  unsigned long mac_adr = (baseMac[2]) + (baseMac[3] * 213) + (baseMac[4] * 231 * 253) + (baseMac[5] * 237 * 219 * 251);
  mac_adr = mac_adr + 1321754191; // Calculate mac_adr
  String sn = convertToArbitraryBase(mac_adr, 68); // Convert to arbitrary base
  if (sn.length() > 5) { // Ensure sn is not longer than 5 characters
    sn = sn.substring(0, 5);
  }
  return sn;
}

String showDateGerman() {
  const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char* monthNumbers[12] = {"01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"};
  String originalDate = __DATE__;
  String monthString = originalDate.substring(0, 3);
  String day = originalDate.substring(4, 6);
  String year = originalDate.substring(9, 11);
  String month = "00";
  for (int i = 0; i < 12; i++) {
    if (monthString == months[i]) {
      month = monthNumbers[i];
      break;
    }
  }
  return day + "." + month + "." + year;
}


