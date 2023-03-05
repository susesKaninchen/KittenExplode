#include <Arduino.h>
#include <WS2812FX.h>



// led pins
#define NUM_LEDS 10
#define DATA_PIN 25

// number of selected resistors
#define MIN_NUM 3
#define MAX_NUM 6

WS2812FX ws2812fx = WS2812FX(NUM_LEDS, DATA_PIN, NEO_GRBW + NEO_KHZ800);

// To-Do: get seed from master module
uint16_t seed = 54321;

// indexes of selected resistors
int selectedResistors[MAX_NUM];

// fills array with one value
void fillArray(int array[], int arraySize, int value) {
  for(int i = 0; i < arraySize; i++) {
    array[i] = value;
  } 
}

// returns true if integer is already in array
bool checkNumInArray(int array[], int arraySize, int checkNum) {
  for(int i = 0; i < arraySize; i++) {
    if(array[i] == checkNum) {
      return true;
    }
  }
  return false;
}

// returns pseudo-random number depending on the given seed
int roundedDigit(int num, int digit) {
  if(digit < 4) {
    return round((num / (10^(digit + 1))) % 10);
  }
  int a = round(num % 10);
  int b = round(num / 10^(digit - 3));
  return round((a*b) % 10);
}

// writes indexes of resistors to be lit up into selsectedResistors array
void selectResistors(uint16_t seed) {
  fillArray(selectedResistors, MAX_NUM, -1);
  // determine pseudorandom number of resistors between MIN_NUM and MAX_NUM values
  int numberOfResistors = round((seed % (MAX_NUM - MIN_NUM + 1))) + MIN_NUM;
  // determine unique, pseudorandom indexes
  for(int i = 0; i < numberOfResistors; i++) {
    int temp = roundedDigit(seed, i);
    while(checkNumInArray(selectedResistors, MAX_NUM, temp)) {
      temp++;
      if(temp == 10) {temp = 0;};
    }
    selectedResistors[i] = temp;
  }
}

// game logic
bool defuseOrBoom(int bridgedResistor, uint16_t seed) {
  return true;
}

// lights up selected resistors
void drawLeds(int array[]) {
  for(int i = 0; i < MAX_NUM; i++) {
    if(array[i] > -1) {
      ws2812fx.setColor(array[i], 0xffffffff);
    }
  }
}

// shuts off all leds
void ledsOff() {
  for(int i = 0; i < NUM_LEDS; i++) {
    ws2812fx.setColor(i, 0x00000000);
  }
}

void setup() {
  Serial.begin(115200);

  ws2812fx.init();
  ws2812fx.setBrightness(255);

  ws2812fx.setSegment(0, 0, 0, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(1, 1, 1, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(2, 2, 2, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(3, 3, 3, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(4, 4, 4, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(5, 5, 5, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(6, 6, 6, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(7, 7, 7, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(8, 8, 8, FX_MODE_STATIC, BLACK, 1000, false);
  ws2812fx.setSegment(9, 9, 9, FX_MODE_STATIC, BLACK, 1000, false);

  ws2812fx.start();
}

void loop() {
  selectResistors(seed);

  for(int i = 0; i < MAX_NUM; i++) {
    Serial.print(selectedResistors[i]);
    Serial.print(" ");
  }
  Serial.println();

  ledsOff();
  drawLeds(selectedResistors);

  ws2812fx.service();
  delay(1000);
  seed = random(10000, 60000);
  
}