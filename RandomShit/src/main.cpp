#include <Arduino.h>
#include <Wire.h>
#include "introducion.h"// Die Anleitung für deine Seite

// Stati, die du seten kannst um dem Master Dinge mitzuteilen.
#define STATUS_NO_INIT 254  // Ich bin noch nicht bereit, bitte initialisiere mich
#define STATUS_OK 0         // Alles okay, alles läuft wie geplant
#define STATUS_WIN 25      // Meine Seite wurde Erfolgreich geschafft
//Alles höhere ist der Fehler counter
volatile byte statusChar = STATUS_NO_INIT; // Am anfang nicht bereit

// Das end Byte wird gesetzt, sobald sich der Spielstatus verändert, 
#define END_IDLE 0
#define END_RUNNING 1
#define END_LOST 2
#define END_WIN 3
volatile byte endByte = END_IDLE;// 0 = Nichts, 1 = läuft, 2 = Verloren, 3 = Gewonnen

volatile int seed = 0;    // Der Seed wird bei der Initialisierung übertragen und soll verschiedene Wege eröffnen

#include "MasterCommunication.h"

#include "FastLED.h"
#include "FastLED_RGBW.h"

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include <INA219_WE.h>

#include "Adafruit_APDS9960.h"
Adafruit_APDS9960 apds;

#include "define.h"

Adafruit_24bargraph bar = Adafruit_24bargraph();
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

INA219_WE ina219 = INA219_WE(&Wire1,0x40);

#define NUM_LEDS 64
#define BRIGHTNESS 60

CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];
CRGB ledRing[10];

int isr_flag = 0;

int buttonStateL = 0;
int buttonStateR = 0;

unsigned long timerRainbow, timerFillHex, timerFillMatrix, timerBar, timerGesture, timePlay;
int modeMatrix = 1;
int modeBar = 1;
int countBar, countMatrix;
boolean flagBar, flagPlay;

const int buzzChan = 0;

//Marco neue Inits Idee
byte gmode4Digit = 0;// 0 = Deaktivirt, 1 = only Numbrs, 2 = Word, 3 = PW
byte gmodeMatrix = 0;// 0 = Deaktivirt, 1 = Snake, 2 = PingPong, 3 = Maze, ... (Tetris, Bomberman, Pacman)
byte gmodeEncoder = 0;// 0 = Deaktivirt, 1 = FindTone, 2 = 1D-Spiel, 3 = Color Riddle

char pwSeg[4] = {' ', ' ', ' ', ' '};// Lösungswort
char segDisplay[4] = {'X', 'X', 'X', 'X'};// Lösungswort
int segSpeed = 500;// time in ms to show next digit
unsigned long segtime = 0;
byte buttonstate = 0;//Bit wise; State,Pause,state2, pause2, ....
byte segWinState = 0;// 0 = läuft, 1 = gewonnen
char digits[] = "0123456789"; // Das Array mit allen Ziffern
char uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // Das Array mit allen Großbuchstaben
char uppercaseAndSpecial[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+-={}[]\\|;:'\"<>,.?/"; // Das Array mit allen Großbuchstaben und Sonderzeichen
char *charArrays[] = {digits, uppercase, uppercaseAndSpecial}; // Das Array, das die drei separate Arrays enthält


void initBuzzer() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  ledcSetup(buzzChan, 2000, 12);
  ledcAttachPin(PIN_BUZZER, buzzChan);
  //attachInterrupt(PIN_GESTURE_INT_MATRIX, interruptRoutine, FALLING);
}

void initMotor() {
  pinMode(PIN_MOT_EN, OUTPUT);
	pinMode(PIN_MOT_DIR, OUTPUT);
  digitalWrite(PIN_MOT_EN, LOW);
	digitalWrite(PIN_MOT_DIR, LOW);
}

void initFastLED() {
  pinMode(PIN_LED_MATRIX, OUTPUT);
	pinMode(PIN_LED_RING, OUTPUT);
  digitalWrite(PIN_LED_MATRIX, LOW);
	digitalWrite(PIN_LED_RING, LOW);
  FastLED.addLeds<UCS1903, PIN_LED_MATRIX, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.addLeds<WS2811, PIN_LED_RING>(ledRing, 10);
	FastLED.setBrightness(BRIGHTNESS);
	FastLED.show();
}

void initTochButtons() {
  pinMode(PIN_TOUCH_L, INPUT);
	pinMode(PIN_TOUCH_R, INPUT);
}

void initJoyStick() {
  pinMode(PIN_JOYSTICK_SW, INPUT);
  analogReadResolution(10);// 10 bit Auflösung
}

void ARDUINO_ISR_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->pressed = true;
}

void initButtons() {
  pinMode(PIN_PUSH_1, INPUT);
  attachInterruptArg(PIN_PUSH_1, isr, &button1, FALLING);
	pinMode(PIN_PUSH_2, INPUT);
  attachInterruptArg(PIN_PUSH_2, isr, &button2, FALLING);
	pinMode(PIN_PUSH_3, INPUT);
  attachInterruptArg(PIN_PUSH_3, isr, &button3, FALLING);
	pinMode(PIN_PUSH_4, INPUT);
  attachInterruptArg(PIN_PUSH_4, isr, &button4, FALLING);
}

void initIna() {
  if(!ina219.init()) {
    Serial.println("INA219 not connected!");
  }
  ina219.setADCMode(SAMPLE_MODE_128);
  ina219.setMeasureMode(CONTINUOUS);
  ina219.setPGain(PG_80);
  ina219.setBusRange(BRNG_16);
}

void initAPDS() {
  if(!apds.begin(10,APDS9960_AGAIN_4X,APDS9960_ADDRESS,&Wire1)) {
    Serial.println("failed to initialize ADPS! Please check your wiring.");
  }  else {
    Serial.println("APDS initialized!");
  }
  apds.enableProximity(true);
  apds.enableGesture(true);
  //set the interrupt threshold to fire when proximity reading goes above 175
  //apds.setProximityInterruptThreshold(0, 175);
  //enable the proximity interrupt
  apds.enableProximityInterrupt();
  apds.setGestureGain(APDS9960_GGAIN_8);
}

void initSegmentDisplay() {
  alpha4.begin(0x70,&Wire1);
  alpha4.writeDigitAscii(0, 'I');
  alpha4.writeDigitAscii(1, 'D');
  alpha4.writeDigitAscii(2, 'L');
  alpha4.writeDigitAscii(3, 'E');
}


void setup() {
	Serial.begin(115200);
  Serial.println();
  initMasterConnection();
  initBuzzer();
	initMotor();
  initFastLED();
	initTochButtons();
  initJoyStick();
	initButtons();
	//pinMode(PIN_GESTURE_INT_MATRIX, INPUT_PULLUP);
  //playInitMelody();
  Wire1.begin(PIN_I2C_MS_SDA, PIN_I2C_MS_SCL);
  bar.begin(0x71,&Wire1);
  bar.setBrightness(8);//Medium
  initSegmentDisplay();
  initIna();
  initAPDS();
  
  //demoCode();
}


void writeSegment() {
  for (u_int8_t n = 0 ; n<4;n++) {
    alpha4.writeDigitAscii(n, segDisplay[n]);
  }
}

unsigned long initSegmentGame(char no1, char no2, char no3, char no4, int speed) {
  unsigned long sovleTimeSeg = 40;
  pwSeg[0] = no1;
  pwSeg[1] = no2;
  pwSeg[2] = no3;
  pwSeg[3] = no4;
  segSpeed = speed;
  sovleTimeSeg = sovleTimeSeg * gmode4Digit * (1000/speed);
  segtime = millis();
  return sovleTimeSeg;
}

char getChar() {
  if (gmode4Digit == 0) {
    return ' ';
  }
  return charArrays[gmode4Digit-1][random(0, strlen(charArrays[gmode4Digit-1]))];
}

void handle_button(Button but,byte index, bool rotate) {
    if (but.pressed){// Wenn der Knopf seit letztem mal gedrückt wurde
    but.pressed =  false;// setze ihn auf nicht gedrückt
    if (but.temp != 0) {// Wenn temp (1 =  aus, 0 = rattert durch) angehalten ist, dann weiter laufen lassen
      but.temp = 0;
    }else {
      but.temp = 1;// wenn nicht anhalten
    }
  }
  if (but.temp == 0 && rotate) {// wenn 0 dann neuen buchstaben rein
    // Rotier flag wird nach allen zurück gesetzt
    segDisplay[index] = getChar();
  }
}

void handleSegGame() {
  bool rotate = false;
  if (segtime + segSpeed < millis()) {
    segtime = millis();// Setze Rotierertimer zurück
    rotate = true;
  }
  handle_button(button1, 0, rotate);
  handle_button(button2, 1, rotate);
  handle_button(button3, 2, rotate);
  handle_button(button4, 3, rotate);
  if (rotate) {
    writeSegment();
  }
  if (segDisplay[0] == pwSeg[0] && segDisplay[1] == pwSeg[1] && segDisplay[2] == pwSeg[2] && segDisplay[3] == pwSeg[3]) {
    segWinState = 1;
  } else {
    segWinState = 0;
  }
}

void fillBar(u_int8_t color, u_int8_t blinkRate) {
  for (uint8_t c = 0; c<=23;c++) {
    bar.setBar(c, color);
  }
  bar.blinkRate(blinkRate);
  bar.writeDisplay();
}

void segWriteStatus() {
  if (endByte == END_RUNNING && statusChar < 25) {
    segDisplay[0] = 'R';
    segDisplay[1] = 'U';
    segDisplay[2] = 'N';
    segDisplay[3] = ' ';
  } else if (endByte == END_LOST) {// Verloren
    segDisplay[0] = 'L';
    segDisplay[1] = 'O';
    segDisplay[2] = 'S';
    segDisplay[3] = 'T';
  } else if (endByte == END_WIN || statusChar >=25) {// Gewonnen
    segDisplay[0] = 'W';
    segDisplay[1] = 'I';
    segDisplay[2] = 'N';
    segDisplay[3] = ' ';
  } else if (endByte == END_IDLE) {// Nichts
    segDisplay[0] = 'I';
    segDisplay[1] = 'D';
    segDisplay[2] = 'L';
    segDisplay[3] = 'E';
  }
  writeSegment();
}


void loop() {
  if (endByte == END_RUNNING) {// Spiel läuft
    if (statusChar >= 25) {
      fillBar(LED_GREEN, HT16K33_BLINK_HALFHZ);
      segWriteStatus();
      //Diese Seite ist schon gelöst
    } else {
      //Do the Game
      handleSegGame();
      if ((!gmode4Digit || segWinState) && (!gmodeMatrix) && (!gmodeEncoder)) {// Win
        statusChar += 25;
      }
    }
  } else if (endByte == END_LOST) {// Verloren
    fillBar(LED_RED, HT16K33_BLINK_2HZ);
    segWriteStatus();
  } else if (endByte == END_WIN) {// Gewonnen
    fillBar(LED_GREEN, HT16K33_BLINK_HALFHZ);
    segWriteStatus();
  } else if (endByte == END_IDLE) {// Nichts
    segWriteStatus();
    //idle();
  }
}


unsigned long getSovleTime(int seed) {// Bitte kurz halten Zeitlich gesehen <1 ms
  unsigned long sovleTime = 10;// 10 sekunden zum klar kommen
  if (seed%2 == 0){
     gmode4Digit = 1;// nur Zahlen
     sovleTime += initSegmentGame('0', '0', '0', '0', 500);// 0000 muss gefunden werden und die buchstaben gehen halbsekündlich durch
  } else if (seed%2 == 1) {
    gmode4Digit = 2;// nur Buchstaben
    sovleTime += initSegmentGame('T', 'E', 'S', 'T', 500);// TEST muss gefunden werden und die buchstaben gehen halbsekündlich durch
  } else {
    gmode4Digit = 3;// Buchstaben und SonderZeichen
    sovleTime += initSegmentGame('[', 'X', 'X', ']', 500);// TEST muss gefunden werden und die buchstaben gehen halbsekündlich durch
  }
  return sovleTime;
}