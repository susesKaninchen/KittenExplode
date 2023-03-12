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
char displaybuffer[4] = {' ', ' ', ' ', ' '};

int buttonStateL = 0;
int buttonStateR = 0;

unsigned long timerRainbow, timerFillHex, timerFillMatrix, timerBar, timerGesture, timePlay;
int modeMatrix = 1;
int modeBar = 1;
int countBar, countMatrix;
boolean flagBar, flagPlay;

const int buzzChan = 0;

void initBuzzer() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  ledcSetup(buzzChan, 2000, 12);
  ledcAttachPin(PIN_BUZZER, buzzChan);
  //attachInterrupt(PIN_GESTURE_INT_MATRIX, interruptRoutine, FALLING);
}

void playInitMelody() {
  ledcWriteTone(buzzChan, 600);
  delay(500);
  ledcWriteTone(buzzChan, 800);
  delay(500);
  ledcWriteTone(buzzChan, 0);
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

void initButtons() {
  pinMode(PIN_PUSH_1, INPUT);
	pinMode(PIN_PUSH_2, INPUT);
	pinMode(PIN_PUSH_3, INPUT);
	pinMode(PIN_PUSH_4, INPUT);
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

void demoCode() {
  
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);

  alpha4.clear();
  alpha4.writeDisplay();

  for (uint8_t b=0; b<24; b++ ){
    if ((b % 3) == 0)  bar.setBar(b, LED_RED);
    if ((b % 3) == 1)  bar.setBar(b, LED_YELLOW);
    if ((b % 3) == 2)  bar.setBar(b, LED_GREEN);
  }
  bar.writeDisplay();
  delay(500);
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
  playInitMelody();
  Wire1.begin(PIN_I2C_MS_SDA, PIN_I2C_MS_SCL);
  bar.begin(0x71,&Wire1);
  bar.setBrightness(8);//Medium
  alpha4.begin(0x70,&Wire1);
  initIna();
  initAPDS();
  
  demoCode();
}

void tickTone() {
  if (timePlay < millis() and flagPlay){
    ledcWriteTone(buzzChan, 0);
    flagPlay = false;
  }
}

void playTone(int frequenz, int playtime) {
  if (!flagPlay) {
    timePlay = millis() + playtime;
    ledcWriteTone(buzzChan, frequenz);
    flagPlay = true;
  }
}

void handleGesture() {
    uint8_t gesture = apds.readGesture();
    if(gesture == APDS9960_DOWN) Serial.println("v");
    if(gesture == APDS9960_UP) Serial.println("^");
    if(gesture == APDS9960_LEFT) Serial.println("<");
    if(gesture == APDS9960_RIGHT) Serial.println(">");
}

void rainbow(){
	static uint8_t hue;
	for(int i = 0; i < 10; i++){
		ledRing[i] = CHSV((i * 256 / 10) + hue, 255, 255);
	}
	FastLED.show();
	hue++;
}

void writehex() {
  char c = random(255);
    if (!isprint(c)) {
      while(!isprint(c)) {
        c = random(255);
      }
    }; // only printable!

   // scroll down display
  displaybuffer[0] = displaybuffer[1];
  displaybuffer[1] = displaybuffer[2];
  displaybuffer[2] = displaybuffer[3];
  displaybuffer[3] = c;
 
  // set every digit to the buffer
  alpha4.writeDigitAscii(0, displaybuffer[0]);
  alpha4.writeDigitAscii(1, displaybuffer[1]);
  alpha4.writeDigitAscii(2, displaybuffer[2]);
  alpha4.writeDigitAscii(3, displaybuffer[3]);
 
  // write it out!
  alpha4.writeDisplay();
}

void barLoop() {
  if (countBar < 24) {
    if (flagBar) {
      bar.setBar(countBar, LED_OFF);
      flagBar = false;
      countBar++;
    } else {
      switch (modeBar) {
      case 1:
        bar.setBar(countBar, LED_GREEN);
        break;
      case 2:
        bar.setBar(countBar, LED_RED);
        break;
      case 3:
        bar.setBar(countBar, LED_YELLOW);
        break;
      default:
        bar.setBar(countBar, LED_GREEN);
        modeBar = 1;
        break;
      }
      flagBar = true;
    }
    bar.writeDisplay();
  }
  if (countBar >= 24) {
    countBar = 0;
    modeBar++;
  }
}

void randFill() {
  int led = random(0, NUM_LEDS);
  int color = random(1, 9);
  switch (color) {
    case 1:
      leds[led] = CRGB::Red;
      break;
    case 2:
      leds[led] = CRGB::Green;
      break;
    case 3:
      leds[led] = CRGB::Blue;
      break;
    case 4:
      leds[led] = CRGBW(0, 0, 0, 255);
      break;
    case 5:
      leds[led] = CRGB::White;
      break;
    case 6:
      leds[led] = CRGB::Aqua;
      break;
    case 7:
      leds[led] = CRGB::HotPink;
      break;
    case 8:
      leds[led] = CRGB::Yellow;
      break;
    default:
      leds[led] = CRGB::Black;
      break;
  }
  FastLED.show();
}

void colorFill() {
  if (countMatrix < NUM_LEDS) {
    switch (modeMatrix) {
      case 1:
        leds[countMatrix] = CRGB::Red;
        break;
      case 2:
        leds[countMatrix] = CRGB::Green;
        break;
      case 3:
        leds[countMatrix] = CRGB::Blue;
        break;
      case 4:
        leds[countMatrix] = CRGBW(0, 0, 0, 255);
        break;
      default:
        leds[countMatrix] = CRGB::Red;
        modeMatrix = 1;
        break;
    }
		FastLED.show();
    countMatrix++;
  } else {
    countMatrix = 0;
    modeMatrix++;
  }
}

void motorInfo() {
  float current_mA = ina219.getCurrent_mA() + 0.2;
  float busVoltage_V = ina219.getBusVoltage_V();
  if (current_mA <= -0.5 or current_mA >= 0.5) {
    if (busVoltage_V < 1.0 and digitalRead(PIN_MOT_EN) == LOW) {
      current_mA = current_mA * 100;
      int motorvalue = map(abs((int) current_mA),0,4000,300,6000);
      playTone(motorvalue,100);
      //Serial.print("Motor: "); Serial.println(motorvalue);
    }
  }
}

void idle() {
  tickTone();
  if (timerFillHex < millis()) {
    timerFillHex = millis() + 500;
    writehex();
  }
  if (timerRainbow < millis()) {
    timerRainbow = millis() + 10;
    rainbow();
  }
  if (timerBar < millis()) {
    timerBar = millis() + 50;
    barLoop();
  }
  if (timerFillMatrix < millis()) {
    timerFillMatrix = millis() + 20;
    randFill();
  }
  if (timerGesture < millis()) {
    timerGesture = millis() + 100;
    motorInfo();
  }
  
  buttonStateL = digitalRead(PIN_TOUCH_L);
  if (buttonStateL == HIGH) {
    digitalWrite(PIN_MOT_EN, HIGH);
  } else {
    digitalWrite(PIN_MOT_EN, LOW);
  }
}

void fillBar(u_int8_t color, u_int8_t blinkRate) {
  for (uint8_t c = 0; c<=23;c++) {
    bar.setBar(c, color);
  }
  bar.blinkRate(blinkRate);
  bar.writeDisplay();
}


void loop() {
  if (endByte == END_RUNNING) {// Spiel läuft
    if (statusChar >= 25) {
      fillBar(LED_GREEN, HT16K33_BLINK_HALFHZ);
      //Diese Seite ist schon gelöst
    } else {

    }
  } else if (endByte == END_LOST) {// Verloren
    fillBar(LED_RED, HT16K33_BLINK_2HZ);
  } else if (endByte == END_WIN) {// Gewonnen
    fillBar(LED_GREEN, HT16K33_BLINK_HALFHZ);
  } else if (endByte == END_IDLE) {// Nichts
    idle();
  }
}