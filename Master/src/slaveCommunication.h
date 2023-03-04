#define ALIVE_PACKAGE 1
#define SEED_PACKAGE 2
#define TEXT_PACKAGE 3
#define END_PACKAGE 4

#define STATUS_NO_INIT 254
#define STATUS_OK 0
#define STATUS_WIN 25
//Alles höhere ist der Fehler counter

#define END_IDLE 0
#define END_RUNNING 1
#define END_LOST 2
#define END_WIN 3
#define END_PAUSED 4

char str[121];

bool getIntroducion(uint8_t I2C_SLAVE) {
  //Serial.print("Starte Text anfrage, bufer ist gerade auf:");
  //Serial.println(Wire.available());
  Wire.beginTransmission(I2C_SLAVE);  // transmit to device #8
  Wire.write(TEXT_PACKAGE);                     // sends one byte
  Wire.endTransmission();             // stop transmitting
  Wire.requestFrom(I2C_SLAVE, 121);
  //Serial.print("Text angefragt, bufer ist gerade auf:");
  //Serial.println(Wire.available());
  memset(str, 0, 121);
  byte cou = 0;
  char tempC = '0';
  for (int gi = 0; gi < 121; gi++) {  // slave may send less than requested
    tempC = Wire.read();
    if (tempC != 255) {
      str[gi] = tempC;
    }else {
      str[gi] = '\0';
      return false;
    }
  }
  return true;
}
void setEnd(uint8_t I2C_SLAVE, byte result) {
  Wire.beginTransmission(I2C_SLAVE);  // transmit to device #8
  Wire.write(END_PACKAGE);                     // sends one byte
  Wire.write(result);
  Wire.endTransmission();             // stop transmitting
}

unsigned long setSeed(uint8_t I2C_SLAVE, int seed) {
  Wire.beginTransmission(I2C_SLAVE);  // transmit to device #8
  Wire.write(SEED_PACKAGE);                     // sends one byte
  Wire.write((byte) seed & 0xFF);// Send LowByte
  Wire.write((byte)(seed >> 8) & 0xFF);// Send HighByte
  if (Wire.endTransmission() != 0) {
    return 0;
  }
  Wire.requestFrom(I2C_SLAVE, 4);
  byte byte4 = Wire.read();
  byte byte3 = Wire.read();
  byte byte2 = Wire.read();
  byte byte1 = Wire.read();
  if (byte1 == 0xFF && byte2 == 0xFF && byte3 == 0xFF && byte4 == 0xFF) {
    return 0;
  }
  unsigned long timeSlave = ((unsigned long)byte1 << 24) | ((unsigned long)byte2 << 16) | ((unsigned long)byte3 << 8) | byte4;
  return timeSlave;
}

byte getSlaveStatus(uint8_t I2C_SLAVE) {
  Wire.beginTransmission(I2C_SLAVE);  // transmit to device #8
  Wire.write(ALIVE_PACKAGE);                     // sends one byte
  byte error = Wire.endTransmission();             // stop transmitting
  if (error!=0) {
    return 255;
  }
  Wire.requestFrom(I2C_SLAVE, 1);
  byte tempRet;
  while (Wire.available()) {
    tempRet = Wire.read();
  }
  return tempRet;     // receive a byte as byte
}

void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

byte getStatusSlaves(){
  byte errors = 0;
  for (uint8_t slaveCount = 0; slaveCount<8;slaveCount++) {
      TCA9548A(slaveCount);
      byte tm = getSlaveStatus(8);
      if (tm >= 254){// no Init/Error
        Serial.print("Port: ");
        Serial.print(slaveCount);
        Serial.print(" nicht erreichbar oder Initialisiert.(Wird Ignoriert):");
        Serial.println(tm);
        tm = 25;
      }
      errors += tm;
    }
    return errors;
}

void setEndSlaves(byte stat){
  for (uint8_t slaveCount = 0; slaveCount<8;slaveCount++) {
      TCA9548A(slaveCount);
      setEnd(8, stat);
    }
}

long initSlaves(int seed) {
  long gameTime = 0;
  for (uint8_t slaveCount = 0; slaveCount<8;slaveCount++) {
    TCA9548A(slaveCount);
    gameTime += setSeed(8, seed);
  }
  byte fin = 10;
  while (fin!=0) {// Wenn init nicht Erfolgreich war, endlosschleife
    delay(10);// Warte, damit nicht anderuend interrupts ausgelöst sind
    fin = 0;
    for (uint8_t slaveCount = 0; slaveCount<8;slaveCount++) {
      TCA9548A(slaveCount);
      byte tm = getSlaveStatus(8);
      if (tm != 255) {// Wenn kein Fehler
        fin +tm;
      }else {
        Serial.print("(Init)Port: ");
        Serial.print(slaveCount);
        Serial.println(" nicht erreichbar.(Wird Ignoriert)");
      }
    }
  }
  return gameTime;
}

String getText(uint8_t bus) {
  String text = "";
  bool endMarker = true;
  TCA9548A(bus);
  do {
    endMarker = getIntroducion(8);
    text = text + str;
  } while (endMarker);
  return text;
}
