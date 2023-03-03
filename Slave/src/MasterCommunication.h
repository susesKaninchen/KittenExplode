
#define ALIVE_PACKAGE 1
#define SEED_PACKAGE 2
#define TEXT_PACKAGE 3
#define END_PACKAGE 4
volatile byte requestType = ALIVE_PACKAGE;
volatile int countText = 0;


void receiveEvent(int howMany) {
  byte c = Wire.read(); // receive Package identyfyer
  if (c == ALIVE_PACKAGE) { // Status abfrage
    Wire.slaveWrite((uint8_t *)&statusChar, 1);
  }
  if (c == SEED_PACKAGE) { // Seed setzen und damit initialisieren
    byte lowbyte = 0;
    while (Wire.available()) {
      byte c = Wire.read();
      seed = (c << 8) | lowbyte;
      lowbyte = c;
    }
    unsigned long sovleTime = 10;// Zeit zum lösen des Rätsels in Sekunden
    Wire.slaveWrite((uint8_t *)&sovleTime, 4);
    statusChar = STATUS_OK;
    endByte = END_RUNNING;
    countText = 0;
  }
  char temp[2];
  temp[0] = 0x03;
  temp[1] = '\0';
  if (c == TEXT_PACKAGE) {//Send Text parts
    int lenthBytes = 0;
    if (countText + 121 >= lenAnleitung) {
      lenthBytes = lenAnleitung - countText;
    }else {
      lenthBytes = 121;
    }
      Wire.slaveWrite((uint8_t *)(anleitung+countText), lenthBytes); // Senden Sie den nächsten 120-Byte-Block
      countText += 121; // Setzen Sie den Startindex für den nächsten 120-Byte-Block
    if (countText >= lenAnleitung) {
      countText = 0;
    }
  }
  if (c == END_PACKAGE) {//Send Text parts
    endByte = Wire.read();
    if (endByte == END_WIN) {
      statusChar += STATUS_WIN;
    }else if (endByte == END_LOST) {
      
    }
  }
}

boolean initMasterConnection() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // function that executes whenever data is received from writer
  return true;
}
