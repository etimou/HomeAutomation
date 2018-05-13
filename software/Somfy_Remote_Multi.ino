/*   This sketch allows you to emulate a Somfy RTS or Simu HZ remote.
   If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/
   
   The rolling code will be stored in EEPROM, so that you can power the Arduino off.
   
   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' to the serial terminal
  To make a group command, just repeat the last two steps with another blind (one by one)
  
  Then:
    - m, u or h will make it to go up
    - s make it stop
    - b, or d will make it to go down
    - you can also send a HEX number directly for any weird command you (0x9 for the sun and wind detector for instance)
*/

#include <EEPROM.h>
//#define PORT_TX 5
#define PIN_TX 2

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8
#define EEPROM_ADDRESS 0

#define REMOTE 0x121300    //<-- Change it!

byte frame[7];
byte checksum;

void BuildFrame(byte *frame, byte button, byte remoteNumber);
void SendCommand(byte *frame, byte sync);


void setup() {
  Serial.begin(115200);
  //DDRD |= 1<<PORT_TX; // Pin 5 an output
  pinMode(PIN_TX, OUTPUT);
  //PORTD &= !(1<<PORT_TX); // Pin 5 LOW
  digitalWrite(PIN_TX, LOW);

  unsigned int code;

  for (int i=0; i<10; i++) {
    Serial.print("Simulated remote number : "); Serial.println(REMOTE+i, HEX);
    EEPROM.get(EEPROM_ADDRESS+2*i, code);
    Serial.print("Current rolling code    : "); Serial.println(code);   
  }
}

void loop() {
  byte remoteNumber=0;
  if (Serial.available() > 1) {//2 bytes should be received
    char serie = (char)Serial.read();
    Serial.println("");
//    Serial.print("Remote : "); Serial.println(REMOTE, HEX);

    if(serie == 'm'||serie == 'u'||serie == 'h') {
      remoteNumber=Serial.read()-48;
      if (remoteNumber > 9) return;
      Serial.print("Remote "); Serial.print(remoteNumber); Serial.print(" ");
      Serial.println("Monte"); // Somfy is a French company, after all.
      BuildFrame(frame, HAUT, remoteNumber);
    }
    else if(serie == 's') {
      remoteNumber=Serial.read()-48;
      if (remoteNumber > 9) return;
      Serial.print("Remote "); Serial.print(remoteNumber); Serial.print(" ");
      Serial.println("Stop");
      BuildFrame(frame, STOP, remoteNumber);
    }
    else if(serie == 'b'||serie == 'd') {
      remoteNumber=Serial.read()-48;
      if (remoteNumber > 9) return;
      Serial.print("Remote "); Serial.print(remoteNumber); Serial.print(" ");
      Serial.println("Descend");
      BuildFrame(frame, BAS, remoteNumber);
    }
    else if(serie == 'p') {
      remoteNumber=Serial.read()-48;
      if (remoteNumber > 9) return;
      Serial.print("Remote "); Serial.print(remoteNumber); Serial.print(" ");
      Serial.println("Prog");
      BuildFrame(frame, PROG, remoteNumber);
    }
    else {
      //Serial.println("Custom code");
      //BuildFrame(frame, serie, 0);
      return;
    }

    Serial.println("Transmit command");
    SendCommand(frame, 2);
    for(int i = 0; i<2; i++) {
      SendCommand(frame, 7);
    }
  }
}


void BuildFrame(byte *frame, byte button, byte remoteNumber) {
  unsigned int code;
  EEPROM.get(EEPROM_ADDRESS+2*remoteNumber, code);
  frame[0] = 0xA7; // Encryption key. Doesn't matter much
  frame[1] = button << 4;  // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = code >> 8;    // Rolling code (big endian)
  frame[3] = code;         // Rolling code
  frame[4] = REMOTE+remoteNumber >> 16; // Remote address
  frame[5] = REMOTE+remoteNumber >>  8; // Remote address
  frame[6] = REMOTE+remoteNumber;       // Remote address

  Serial.print("Frame         : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      Serial.print("0");     // nibble is a 0.
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  
// Checksum calculation: a XOR of all the nibbles
  checksum = 0;
  for(byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only


//Checksum integration
  frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
                        // consider the checksum ok.

  Serial.println(""); Serial.print("With checksum : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }

  
// Obfuscation: a XOR of all the bytes
  for(byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i-1];
  }

  Serial.println(""); Serial.print("Obfuscated    : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Rolling Code  : "); Serial.println(code);
  EEPROM.put(EEPROM_ADDRESS+2*remoteNumber, code + 1); //  We store the value of the rolling code in the
                                        // EEPROM. It should take up to 2 adresses but the
                                        // Arduino function takes care of it.
}

void SendCommand(byte *frame, byte sync) {
  if(sync == 2) { // Only with the first frame.
  //Wake-up pulse & Silence
    //PORTD |= 1<<PORT_TX;
    digitalWrite(PIN_TX, HIGH);
    delayMicroseconds(9415);
    //PORTD &= !(1<<PORT_TX);
    digitalWrite(PIN_TX, LOW);
    delayMicroseconds(89565);
  }

// Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    //PORTD |= 1<<PORT_TX;
    digitalWrite(PIN_TX, HIGH);
    delayMicroseconds(4*SYMBOL);
    //PORTD &= !(1<<PORT_TX);
    digitalWrite(PIN_TX, LOW);
    delayMicroseconds(4*SYMBOL);
  }

// Software sync
  //PORTD |= 1<<PORT_TX;
  digitalWrite(PIN_TX, HIGH);
  delayMicroseconds(4550);
  //PORTD &= !(1<<PORT_TX);
  digitalWrite(PIN_TX, LOW);
  delayMicroseconds(SYMBOL);
  
  
//Data: bits are sent one by one, starting with the MSB.
  for(byte i = 0; i < 56; i++) {
    if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
      //PORTD &= !(1<<PORT_TX);
      digitalWrite(PIN_TX, LOW);
      delayMicroseconds(SYMBOL);
      //PORTD ^= 1<<5;
      digitalWrite(PIN_TX, HIGH);
      delayMicroseconds(SYMBOL);
    }
    else {
      //PORTD |= (1<<PORT_TX);
      digitalWrite(PIN_TX, HIGH);
      delayMicroseconds(SYMBOL);
      //PORTD ^= 1<<5;
      digitalWrite(PIN_TX, LOW);
      delayMicroseconds(SYMBOL);
    }
  }
  
  //PORTD &= !(1<<PORT_TX);
  digitalWrite(PIN_TX, LOW);
  delayMicroseconds(30415); // Inter-frame silence
}

