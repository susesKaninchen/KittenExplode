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

int seedGobal = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Wire.begin(); // join i2c bus (address optional for writer)
  IPAddress apIP(192,1,1,1);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  SPIFFS.begin();
    server.on("/init", HTTP_POST, [](AsyncWebServerRequest *request){
      if(request->hasParam("zahl")){
        AsyncWebParameter* p = request->getParam("zahl");
        seedGobal = atoi(p->value().c_str());
      }
        request->redirect("/anleitung.html");
    });
    server.serveStatic("/img/", SPIFFS, "/img/");
    server.serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);
    
    server.onNotFound(notFound);
    server.begin();
  Serial.println("Master getartet");
}

void loop() {
  Serial.println("Starte Spiel");
  Serial.print("Initiire Slaves mit Seed:");
  Serial.println(8);
  unsigned long timeGame = initSlaves(8);
  Serial.print("Das Spiel dauert: ");
  Serial.print(timeGame);
  Serial.println(" Sekunden");
  Serial.print("Status: ");
  Serial.println(getStatusSlaves());
  Serial.println("Starte Slaves");
  setEndSlaves(END_RUNNING);
  bool gameRunning = true;
  byte gameStati = 0;
  unsigned long startPoint = millis();
  unsigned long lastWrite = millis();
  while (gameRunning) {
    gameStati = getStatusSlaves();
    if (millis() > lastWrite + 2000) {
      Serial.print("Status\nFehler: ");
      Serial.println(gameStati%25);
      Serial.print("Seiten Geschafft: ");
      Serial.println((int) gameStati/25);
      Serial.print("Rest Zeit: ");
      Serial.println(startPoint + timeGame * 1000 - millis());// wenn -, dann sehr hoche zahl
      lastWrite = millis();
    }
    if (gameStati >= 200) {
      setEndSlaves(END_WIN);
      Serial.println("Gewonnen");
      gameRunning = false;
    }else if (gameStati%25 >= 3) {
      setEndSlaves(END_LOST);
      Serial.println("Verloren zu viele Fehler");
      gameRunning = false;
    } else if (millis() > startPoint + timeGame * 1000) {
      setEndSlaves(END_LOST);
      Serial.println("Verloren Zeit ist um");
      gameRunning = false;
    }
    delay(1000);
  }
  delay(10000);
}
