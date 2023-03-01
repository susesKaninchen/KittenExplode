#include "introducion.h"// Die Anleitung für deine Seite
#include <Wire.h>       // Die Libary für die Verbindung
#define LED_BUILTIN 2   // Onbord LED, fals du sie benutzen möchtest

// Stati, die du seten kannst um dem Master Dinge mitzuteilen.
#define STATUS_NO_INIT 254  // Ich bin noch nicht bereit, bitte initialisiere mich
#define STATUS_OK 0         // Alles okay, alles läuft wie geplant
#define STATUS_WIN 200      // Meine Seite wurde Erfolgreich geschafft
//Alles höhere ist der Fehler counter
volatile byte statusChar = STATUS_NO_INIT; // Am anfang nicht bereit

// Das end Byte wird gesetzt, sobald sich der Spielstatus verändert, 
#define END_IDLE 0
#define END_RUNNING 1
#define END_LOST 2
#define END_WIN 3
volatile byte endByte = END_IDLE;// 0 = Nichts, 1 = läuft, 2 = Verloren, 3 = Gewonnen

volatile int seed = 0;    // Der Seed wird bei der Initialisierung übertragen und soll verschiedene Wege eröffnen

#include "MasterCommunication.h";

void setup() {
  initMasterConnection();
  //Serial.begin(115200);// Optional um dinge auszugeben, beim Debuggen
}

void loop() {
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  while (endByte == END_RUNNING) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    statusChar = 25;
    // Aktionen machen und alles was du möchtest
    // Code am besten non Blocking (max laufzeit < 1000 ms)
  }
  while (endByte == END_LOST) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    // Verloren Animation anzeigen
  }
  while (endByte == END_WIN) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    // Win Animation anzeigen
  }
}
