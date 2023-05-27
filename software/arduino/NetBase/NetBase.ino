#include <EEPROM.h>

const byte numChars = 128;
char receivedChars[numChars];   // an array to store the received data
char tempChars[numChars];        // temporary array for use when parsing
byte frame[7]; // frame for Somfy protocol
unsigned char msg_counter = 0;

unsigned char protocolName = 0; //data to be parsed
unsigned long deviceAddress = 0;
unsigned char buttonNumber = 0;
unsigned char actionCommand = 0;

bool PIRstatus = 0;
bool edgeDetected = 0;
unsigned long  edgeDetectedTime = 0;

#define PROTOCOL_SOMFY 1
#define PROTOCOL_HOMEEASY 2
#define PROTOCOL_NEWKAKU 3
#define PROTOCOL_PING 4
#define UP 0x2
#define STOP 0x1
#define DOWN 0x4
#define PROG 0x8
#define ON 0x3
#define OFF 0x5

#define PIN_TXS 4
#define PIN_TX 5
#define PIN_ALARM 7
#define PIN_PIR 6
#define RETRIES 5
#define EEPROM_ADDRESS 0
#define REMOTE 0x121300    //<-- Change it!
#define SYMBOL 640



void setup() {
  Serial.begin(57600);

  pinMode(PIN_TX, OUTPUT);
  digitalWrite(PIN_TX, LOW);
  pinMode(PIN_TXS, OUTPUT);
  digitalWrite(PIN_TXS, LOW);
  pinMode(PIN_ALARM, OUTPUT);
  digitalWrite(PIN_ALARM, LOW);
  pinMode(PIN_PIR, INPUT_PULLUP);

  // Welcome message
  Serial.println("20;00;Etimou RadioFrequencyLink - RFLink Gateway Clone V1.0;");
}

void loop() {

  /*main loop, waiting for events*/

  if (recvWithEndMarker()) { //event from serial
    if (parseData() != -1) {
      processCommand();
    }
  }

  processLocal();

}

bool recvWithEndMarker() {
  static byte ndx = 0;
  char rc;
  boolean newData = false;

  while (Serial.available() > 0) {
    rc = Serial.read();

    if (rc != '\n' && rc != '\r') {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      if (ndx > 0) {
        receivedChars[ndx] = '\0'; // terminate the string
        ndx = 0;
        newData = true;
        //Serial.println(receivedChars);
      }
    }
  }
  return newData;
}

int parseData() {

  protocolName = 0; //set default values before start
  deviceAddress = 0;
  buttonNumber = 0;
  actionCommand = 0;

  strcpy(tempChars, receivedChars); //copy the received string

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strstr(tempChars, "10");
  if (strlen(strtokIndx) <= 2) {
    return -1;
  }


  strtokIndx = strtok(strtokIndx, ";");     // get the first part
  if (strtokIndx == NULL)
    return -1;


  strtokIndx = strtok(NULL, ";");
  if (strtokIndx == NULL)
    return -1;


  if (!strcmp(strtokIndx, "RTS")) protocolName = PROTOCOL_SOMFY;
  else if (!strcmp(strtokIndx, "X10")) protocolName = PROTOCOL_SOMFY;
  else if (!strcmp(strtokIndx, "HomeEasy")) protocolName = PROTOCOL_HOMEEASY;
  else if (!strcmp(strtokIndx, "NewKaku")) protocolName = PROTOCOL_NEWKAKU;
  else if (!strcmp(strtokIndx, "Kaku")) protocolName = PROTOCOL_NEWKAKU;
  else if (!strcmp(strtokIndx, "PING")) {
    protocolName = PROTOCOL_PING;
    return 0;
  }
  else return -1;

  strtokIndx = strtok(NULL, ";");
  if (strtokIndx == NULL)
    return -1;

  deviceAddress = strtol(strtokIndx, NULL, 16);
  if (deviceAddress == 0)
    return -1;
  if (protocolName == PROTOCOL_HOMEEASY) {
    deviceAddress = deviceAddress & 0x00FFFFFF;
  }

  strtokIndx = strtok(NULL, ";");
  if (strtokIndx == NULL)
    return -1;

  buttonNumber = (int)strtol(strtokIndx, NULL, 16) - 1;

  strtokIndx = strtok(NULL, ";");
  if (strtokIndx == NULL)
    return -1;


  if (!strcmp(strtokIndx, "ON")) actionCommand = ON;
  else if (!strcmp(strtokIndx, "OFF")) actionCommand = OFF;
  else if (!strcmp(strtokIndx, "UP")) actionCommand = UP;
  else if (!strcmp(strtokIndx, "DOWN")) actionCommand = DOWN;
  else if (!strcmp(strtokIndx, "STOP")) actionCommand = STOP;
  else if (!strcmp(strtokIndx, "PROG")) actionCommand = PROG;
  else return -1;

  return 0;


}



void processCommand() {

  if (protocolName == PROTOCOL_HOMEEASY) {
    sendHomeEasy();
  }
  else if (protocolName == PROTOCOL_NEWKAKU) {
    if (deviceAddress == 0xFFFFFF) {
      digitalWrite(PIN_ALARM, actionCommand == ON);
    }
    else if (deviceAddress == 0xFFFFFE) {
      if (actionCommand == ON)
      {
        digitalWrite(PIN_ALARM, HIGH);
        delay(150);
        digitalWrite(PIN_ALARM, LOW);
        delay(300);
        digitalWrite(PIN_ALARM, HIGH);
        delay(150);
        digitalWrite(PIN_ALARM, LOW);
      }
      else {
        digitalWrite(PIN_ALARM, HIGH);
        delay(150);
        digitalWrite(PIN_ALARM, LOW);
      }
    }
    else {
      if (actionCommand == ON) sendSimple433();
    }
  }
  else if (protocolName == PROTOCOL_SOMFY) {
    sendSomfy();
  }

  Serial.print("20;");
  Serial.print(msg_counter++, HEX);
  Serial.print(";OK;");
  Serial.println();
}

void sendSimple433()
{
  int dataLength = 24;
  for (int n = 0; n < RETRIES; n++)
  {
    digitalWrite(PIN_TX, HIGH);
    delayMicroseconds(510);
    digitalWrite(PIN_TX, LOW);
    delayMicroseconds(2040);


    for (int i = dataLength - 1; i >= 0; i--) {
      if (deviceAddress & (1L << i))
      {
        digitalWrite(PIN_TX, HIGH);
        delayMicroseconds(1020);
        digitalWrite(PIN_TX, LOW);
        delayMicroseconds(510);
      }
      else
      {
        digitalWrite(PIN_TX, HIGH);
        delayMicroseconds(510);
        digitalWrite(PIN_TX, LOW);
        delayMicroseconds(1020);
      }
    }
  }
}

void sendHomeEasyOne()
{
  digitalWrite(PIN_TX, HIGH);
  delayMicroseconds(275);
  digitalWrite(PIN_TX, LOW);
  delayMicroseconds(1225);
  digitalWrite(PIN_TX, HIGH);
  delayMicroseconds(275);
  digitalWrite(PIN_TX, LOW);
  delayMicroseconds(275);
}
void sendHomeEasyZero()
{
  digitalWrite(PIN_TX, HIGH);
  delayMicroseconds(275);
  digitalWrite(PIN_TX, LOW);
  delayMicroseconds(275);
  digitalWrite(PIN_TX, HIGH);
  delayMicroseconds(275);
  digitalWrite(PIN_TX, LOW);
  delayMicroseconds(1225);
}



void sendHomeEasy()
{
  for (int n = 0; n < RETRIES; n++)
  {
    digitalWrite(PIN_TX, HIGH);
    delayMicroseconds(275);
    digitalWrite(PIN_TX, LOW);
    delayMicroseconds(2675);

    //address
    for (int i = 25; i >= 0; i--) {
      if (deviceAddress & (1L << i)) sendHomeEasyOne();
      else sendHomeEasyZero();

    }
    //group flag, not managed
    sendHomeEasyZero();

    //command 0 or 1
    if (actionCommand == ON) sendHomeEasyOne();
    else sendHomeEasyZero();

    //device code
    for (int i = 3; i >= 0; i--) {
      if (buttonNumber & (1L << i)) sendHomeEasyOne();
      else sendHomeEasyZero();

    }
    //end
    digitalWrite(PIN_TX, HIGH);
    delayMicroseconds(275);
    digitalWrite(PIN_TX, LOW);

    delay(10);
  }
}

void buildFrameSomfy() {
  unsigned int Code;
  EEPROM.get(EEPROM_ADDRESS + 2 * buttonNumber, Code);
  frame[0] = 0xA7; // Encryption key. Doesn't matter much
  frame[1] = actionCommand << 4;  // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = Code >> 8;    // Rolling code (big endian)
  frame[3] = Code;         // Rolling code
  frame[4] = REMOTE + buttonNumber >> 16; // Remote address
  frame[5] = REMOTE + buttonNumber >>  8; // Remote address
  frame[6] = REMOTE + buttonNumber;     // Remote address

  //Serial.print("Frame         : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      //Serial.print("0");     // nibble is a 0.
    }
    //Serial.print(frame[i],HEX); Serial.print(" ");
  }

  // Checksum calculation: a XOR of all the nibbles
  byte checksum = 0;
  for (byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only


  //Checksum integration
  frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
  // consider the checksum ok.

  //Serial.println(""); Serial.print("With checksum : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {
      //Serial.print("0");
    }
    //Serial.print(frame[i],HEX); Serial.print(" ");
  }


  // Obfuscation: a XOR of all the bytes
  for (byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i - 1];
  }

  //Serial.println(""); Serial.print("Obfuscated    : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {
      //Serial.print("0");
    }
    //Serial.print(frame[i],HEX); Serial.print(" ");
  }
  //Serial.println("");
  //Serial.print("Rolling Code  : "); Serial.println(code);
  EEPROM.put(EEPROM_ADDRESS + 2 * buttonNumber, Code + 1); //  We store the value of the rolling code in the
  // EEPROM. It should take up to 2 adresses but the
  // Arduino function takes care of it.
}

void sendCommandSomfy(byte sync) {
  if (sync == 2) { // Only with the first frame.
    //Wake-up pulse & Silence
    digitalWrite(PIN_TXS, HIGH);
    delayMicroseconds(9415);
    digitalWrite(PIN_TXS, LOW);
    delayMicroseconds(89565);
  }

  // Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    digitalWrite(PIN_TXS, HIGH);
    delayMicroseconds(4 * SYMBOL);
    digitalWrite(PIN_TXS, LOW);
    delayMicroseconds(4 * SYMBOL);
  }

  // Software sync
  digitalWrite(PIN_TXS, HIGH);
  delayMicroseconds(4550);
  digitalWrite(PIN_TXS, LOW);
  delayMicroseconds(SYMBOL);


  //Data: bits are sent one by one, starting with the MSB.
  for (byte i = 0; i < 56; i++) {
    if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1) {
      digitalWrite(PIN_TXS, LOW);
      delayMicroseconds(SYMBOL);
      digitalWrite(PIN_TXS, HIGH);
      delayMicroseconds(SYMBOL);
    }
    else {
      digitalWrite(PIN_TXS, HIGH);
      delayMicroseconds(SYMBOL);
      digitalWrite(PIN_TXS, LOW);
      delayMicroseconds(SYMBOL);
    }
  }

  digitalWrite(PIN_TXS, LOW);
  delayMicroseconds(30415); // Inter-frame silence
}

void sendSomfy() {

  buildFrameSomfy();
  sendCommandSomfy(2);
  for (int i = 0; i < 2; i++) {
    sendCommandSomfy(7);
  }


}
//void processLocal(){
//
//  if (digitalRead(PIN_PIR)!=PIRstatus)
//  {
//    PIRstatus = !PIRstatus;
//
//    Serial.print("20;");
//    Serial.print(msg_counter++, HEX);
//    Serial.print(";Kaku;");
//
//    Serial.print("ID=");
//    Serial.print("FFFFFD");
//    Serial.print(";");
//
//
//    Serial.print("SWITCH=");
//    Serial.print("1");
//    Serial.print(";");
//
//
//    Serial.print("CMD=");
//    if (PIRstatus) Serial.print("01");
//    else Serial.print("00");
//
//    Serial.println("0000");
//  }
//}
void processLocal() {

  if (!edgeDetected) {
    if (digitalRead(PIN_PIR) != PIRstatus) {
      edgeDetected = 1;
      edgeDetectedTime = millis();
    }
    return;
  }
  else if (edgeDetected) {
    if (millis() - edgeDetectedTime > 2000) {
      if (digitalRead(PIN_PIR) != PIRstatus) {
        //change confirmed
        PIRstatus = !PIRstatus;
        edgeDetected = 0;


        Serial.print("20;");
        Serial.print(msg_counter++, HEX);
        Serial.print(";Kaku;");

        Serial.print("ID=");
        Serial.print("FFFFFD");
        Serial.print(";");


        Serial.print("SWITCH=");
        Serial.print("1");
        Serial.print(";");


        Serial.print("CMD=");
        if (PIRstatus) Serial.print("01");
        else Serial.print("00");

        Serial.println("0000");


      }
      else {
        //change not confirmed
        edgeDetected = 0;
      }
    }
  }
}
