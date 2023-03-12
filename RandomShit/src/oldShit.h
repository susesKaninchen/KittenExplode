
void playInitMelody() {
  ledcWriteTone(buzzChan, 600);
  delay(500);
  ledcWriteTone(buzzChan, 800);
  delay(500);
  ledcWriteTone(buzzChan, 0);
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