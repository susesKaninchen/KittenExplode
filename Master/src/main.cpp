#include "soc/rtc_wdt.h"

#include <Arduino.h>
#include "introducion.h"
#include <Wire.h>
#define LED_BUILTIN 2
#include "slaveCommunication.h"

#include <FS.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>

#include <TM1637.h>
// Instantiation and pins configurations
// Pin 19 - > DIO
// Pin 18 - > CLK
TM1637 tm(18, 19);
LiquidCrystal_I2C lcd(0x27, 20, 4); 
int seedGobal = 0;
unsigned long timeGame = -1;
bool gameRunning = false;
byte gameStati = 0;
unsigned long startPoint = millis();
unsigned long lastWrite = millis();
int minuten = 0;
int sekunden = 0;
int millisekunden = 0;
unsigned long ts = 0;
uint8_t bombb[8] = {0x1, 0x2, 0x4, 0xe, 0x11, 0x11, 0x11, 0xe};


#include <Adafruit_NeoPixel.h>
#define LED_PIN        26
#define NUMPIXELS 6
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);

const char* ssid = "Kitten";
const char* password = "";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String processor(const String& var)
{
  if(var == "ANLEITUNG_1") {
    return getText(0);
  } else if (var == "ANLEITUNG_2") {
    return getText(1);
  } else if (var == "ANLEITUNG_3") {
    return getText(2);
  } else if (var == "ANLEITUNG_4") {
    return getText(3);
  } else if (var == "ANLEITUNG_5") {
    return getText(4);
  } else if (var == "ANLEITUNG_6") {
    return getText(5);
  } else if (var == "ANLEITUNG_7") {
    return getText(6);
  } else if (var == "ANLEITUNG_8") {
    return getText(7);
  } else if (var == "") {
    return "%";
  }
  return String("%" + var + "%");
}

void setPixel(byte error) {
  for (int g = 0 ; g<3; g++) {
    if (error >= g+1){
      pixels.setPixelColor(g, pixels.Color(150, 0, 0));
    } else {
    pixels.setPixelColor(g, pixels.Color(0, 150, 0));
  }
  }
  pixels.show();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, bombb);
  lcd.home();
  lcd.setCursor(0,0);
  lcd.print("Starte Spiel");
  lcd.setCursor(0,1);
  lcd.print("ueber 192.1.1.1");
  lcd.setCursor(0,3);
  lcd.print("Erfolg! Have fun:)");
  delay(2000);
  lcd.clear();
  lcd.setCursor(3,2);
  lcd.write(0);
  for (int g = 0 ; g<20; g++) {
    lcd.setCursor(4,1);
    lcd.print("------------*");
  }
  lcd.setCursor(17,0);
  lcd.print(",");
  lcd.setCursor(15,2);
  lcd.print("'");

  Wire.begin(); // join i2c bus (address optional for writer)

  IPAddress apIP(192,1,1,1);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  SPIFFS.begin();
  
  server.on("/init", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("zahl")){
      AsyncWebParameter* p = request->getParam("zahl");
      seedGobal = atoi(p->value().c_str());
      timeGame = initSlaves(seedGobal);
      setEndSlaves(END_RUNNING);
      gameRunning = true;
      gameStati = 0;
      startPoint = millis();
      lastWrite = millis();
      minuten = 0;
      sekunden = 0;
      ts = 0;
    }
      request->redirect("/anleitung.html");
  });
  server.serveStatic("/img/", SPIFFS, "/img/");
  server.serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);
  
  server.onNotFound(notFound);
  server.begin();
  
  tm.begin();
  tm.setBrightness(4);
  tm.display(1111)->blink(1000);
  pixels.begin();
  pixels.clear();
  Serial.println("Master getartet");
}

void loop() {
  tm.display("IDLE")->scrollLeft(500);
  while (gameRunning) {
    //tell the wdt we are still alive
    rtc_wdt_feed();// Eigentlich unnötig, da der Watchdog ja gefüttert wird, wenn der irgendeine aktion macht.
    //tm.clearScreen();
    //ts = (int)(startPoint + timeGame * 1000 - millis())/1000;
    //sekunden = (ts)%60;
    //minUten = ts/60;
    //tm.display((float) minUten + (sekunden/100));
    minuten = ((ts- millis()) / (60*1000));
    sekunden = ((ts - millis() - minuten * 60 * 1000) / 1000);
    millisekunden = ((ts - millis() - minuten * 60 * 1000 - sekunden * 1000));
    lcd.setCursor(0,0);
    lcd.print("Zeit: ");
    lcd.setCursor(3,1);
    if (minuten < 10) {
      lcd.print("0");
    }
    lcd.print(minuten);
    lcd.print(":");

    if (sekunden < 10) {
      lcd.print("0");
    }
    lcd.print(sekunden);
    lcd.print(":");

    if (millisekunden < 10) {
      lcd.print("00");
    } else if (millisekunden < 100) {
      lcd.print("0");
    }
    lcd.print(millisekunden);
    lcd.setCursor(0,1);



    gameStati = getStatusSlaves();
    if (millis() > lastWrite + 2000) {
      setPixel(gameStati%25);
      Serial.print("Status\nFehler: ");
      Serial.println(gameStati%25);
      Serial.print("Seiten Geschafft: ");
      Serial.println((int) gameStati/25);
      Serial.print("Rest Zeit: ");
      Serial.println(startPoint + timeGame * 1000 - millis());// wenn -, dann sehr hoche zahl
      lastWrite = millis();
    }
    if (gameStati >= 200) {
      setPixel(0);
      setEndSlaves(END_WIN);
      Serial.println("Gewonnen");
      gameRunning = false;
    }else if (gameStati%25 >= 3) {
      setPixel(3);
      setEndSlaves(END_LOST);
      Serial.println("Verloren zu viele Fehler");
      gameRunning = false;
    } else if (millis() > startPoint + timeGame * 1000) {
      setPixel(3);
      setEndSlaves(END_LOST);
      Serial.println("Verloren Zeit ist um");
      gameRunning = false;
    }
    delay(1000);
  }
  delay(1000);

}
