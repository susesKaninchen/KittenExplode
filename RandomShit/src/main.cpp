#include <Arduino.h>
#include <Wire.h>

#include "FastLED.h"
#include "FastLED_RGBW.h"

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include <INA219_WE.h>

#include "Adafruit_APDS9960.h"
Adafruit_APDS9960 apds;

#include "define.h";

Adafruit_24bargraph bar = Adafruit_24bargraph();
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

INA219_WE ina219 = INA219_WE(&Wire1,0x40);

#define NUM_LEDS 64
CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];
CRGB ledRing[10];

const uint8_t brightness = 60;
int isr_flag = 0;
char displaybuffer[4] = {' ', ' ', ' ', ' '};

int buttonState = 0;

void handleGesture() {
    uint8_t gesture = apds.readGesture();
    if(gesture == APDS9960_DOWN) Serial.println("v");
    if(gesture == APDS9960_UP) Serial.println("^");
    if(gesture == APDS9960_LEFT) Serial.println("<");
    if(gesture == APDS9960_RIGHT) Serial.println(">");
}

/*
void interruptRoutine() {
    detachInterrupt(PIN_GESTURE_INT_MATRIX);
    //handleGesture();
    isr_flag = 1;
    attachInterrupt(PIN_GESTURE_INT_MATRIX, interruptRoutine, FALLING);
}
*/
//void IRAM_ATTR interruptRoutine() {
//    isr_flag = 1;
//}

void setup() {
	Serial.begin(115200);
	
  delay(5000);
  Serial.println();

	pinMode(PIN_BUZZER, OUTPUT);
	pinMode(PIN_MOT_EN, OUTPUT);
	pinMode(PIN_MOT_DIR, OUTPUT);
	pinMode(PIN_LED_MATRIX, OUTPUT);
	pinMode(PIN_LED_RING, OUTPUT);
	
	digitalWrite(PIN_BUZZER, LOW);
	digitalWrite(PIN_MOT_EN, LOW);
	digitalWrite(PIN_MOT_DIR, LOW);
	digitalWrite(PIN_LED_MATRIX, LOW);
	digitalWrite(PIN_LED_RING, LOW);
	
	pinMode(PIN_TOUCH_L, INPUT);
	pinMode(PIN_TOUCH_R, INPUT);
	pinMode(PIN_JOYSTICK_SW, INPUT);
	pinMode(PIN_PUSH_1, INPUT);
	pinMode(PIN_PUSH_2, INPUT);
	pinMode(PIN_PUSH_3, INPUT);
	pinMode(PIN_PUSH_4, INPUT);
	pinMode(PIN_GESTURE_INT_MATRIX, INPUT_PULLUP);

  //attachInterrupt(PIN_GESTURE_INT_MATRIX, interruptRoutine, FALLING);


	FastLED.addLeds<UCS1903, PIN_LED_MATRIX, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.addLeds<WS2811, PIN_LED_RING>(ledRing, 10);
	FastLED.setBrightness(brightness);
	FastLED.show();
  
  Wire1.begin(PIN_I2C_MS_SDA, PIN_I2C_MS_SCL);
  bar.begin(0x71,&Wire1);
  alpha4.begin(0x70,&Wire1);
  
  if(!ina219.init()) {
    Serial.println("INA219 not connected!");
  }

  if(!apds.begin(10,APDS9960_AGAIN_4X,APDS9960_ADDRESS,&Wire1)) {
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");

  apds.enableProximity(true);
  apds.enableGesture(true);

  //set the interrupt threshold to fire when proximity reading goes above 175
  //apds.setProximityInterruptThreshold(0, 175);
  //enable the proximity interrupt
  //apds.enableProximityInterrupt();
  //apds.setGestureGain(APDS9960_GGAIN_4);

  ina219.setADCMode(SAMPLE_MODE_128);
  ina219.setMeasureMode(CONTINUOUS);
  ina219.setPGain(PG_80);
  ina219.setBusRange(BRNG_16);
  //ina219.setShuntVoltOffset(-0.02)

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
  delay(2000);

}
void colorFill(CRGB c){
	for(int i = 0; i < NUM_LEDS; i++){
		leds[i] = c;
		FastLED.show();
		delay(20);
	}
	delay(200);
}
void fillWhite(){
	for(int i = 0; i < NUM_LEDS; i++){
		leds[i] = CRGBW(0, 0, 0, 255);
		FastLED.show();
		delay(20);
	}
	delay(200);
}
void rainbow(){
	static uint8_t hue;
	for(int i = 0; i < 10; i++){
		ledRing[i] = CHSV((i * 256 / 10) + hue, 255, 255);
	}
	FastLED.show();
	hue++;
}
void rainbowLoop(){
	long millisIn = millis();
	long loopTime = 2000; // 5 seconds
	while(millis() < millisIn + loopTime){
		rainbow();
		delay(5);
	}
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

  rainbowLoop();
  //handleGesture();
}

void loop() {
  /*
  if(!digitalRead(PIN_GESTURE_INT_MATRIX)){
    Serial.println(apds.readProximity());
    Serial.println("Interrupt loop called!");
    //handleGesture();
    //clear the interrupt
    apds.clearInterrupt();
  }
  buttonState = digitalRead(PIN_TOUCH_L);
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(PIN_MOT_EN, HIGH);
  } else {
    // turn LED off:
    digitalWrite(PIN_MOT_EN, LOW);
  }
  */
  //handleGesture();
  //delay(500);
  /*
  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0; 
  bool ina219_overflow = false;
  
  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
  ina219_overflow = ina219.getOverflow();
  
  Serial.print("Shunt Voltage [mV]: "); Serial.println(shuntVoltage_mV);
  Serial.print("Bus Voltage [V]: "); Serial.println(busVoltage_V);
  Serial.print("Load Voltage [V]: "); Serial.println(loadVoltage_V);
  Serial.print("Current[mA]: "); Serial.println(current_mA);
  Serial.print("Bus Power [mW]: "); Serial.println(power_mW);
  if(!ina219_overflow){
    Serial.println("Values OK - no overflow");
  }
  else{
    Serial.println("Overflow! Choose higher PGAIN");
  }
  Serial.println();
  
  delay(500);
  */
  //if( isr_flag == 1 ) {
    //detachInterrupt(PIN_GESTURE_INT_MATRIX);
    //Serial.println("Interrupt called!");
    //isr_flag = 0;
    //attachInterrupt(PIN_GESTURE_INT_MATRIX, interruptRoutine, FALLING);
  //}
  
  
 for (uint8_t b=0; b<24; b++) {
   bar.setBar(b, LED_RED);
   bar.writeDisplay();
   delay(50);
   bar.setBar(b, LED_OFF);
   bar.writeDisplay();
 }
 writehex();
 colorFill(CRGB::Red);
  for (uint8_t b=0; b<24; b++) {
   bar.setBar(b, LED_GREEN);
   bar.writeDisplay();
   delay(50);
   bar.setBar(b, LED_OFF);
   bar.writeDisplay();
 }
writehex();
colorFill(CRGB::Green);
 for (uint8_t b=0; b<24; b++) {
   bar.setBar(23-b, LED_YELLOW);
   bar.writeDisplay();
   delay(50);
   bar.setBar(23-b, LED_OFF);
   bar.writeDisplay();
 }
writehex();
colorFill(CRGB::Blue);
fillWhite();

  //handleGesture();
}