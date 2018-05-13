


/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"
#include <EEPROM.h>
#include <RCSwitch.h>

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
#define RETRIES 5
#define EEPROM_ADDRESS 0
#define REMOTE 0x121300    //<-- Change it!
#define SYMBOL 640


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(10,9);
/**********************************************************/

RCSwitch mySwitch = RCSwitch();

byte addresses[6] = "1Node";
unsigned char msg_counter=0;

char cmd[128];
char nbReadBytes=0;
char protocol=0;
unsigned long address=0;
int code=0;
char command=0;
byte frame[7];
byte checksum;


void setup() {
  Serial.begin(57600);
  //printf_begin();
  pinMode(PIN_TX, OUTPUT);
  digitalWrite(PIN_TX, LOW);
  pinMode(PIN_TXS, OUTPUT);
  digitalWrite(PIN_TXS, LOW);
  pinMode(PIN_ALARM, OUTPUT);
  digitalWrite(PIN_ALARM, LOW);

  unsigned int code;

  for (int i=0; i<16; i++) {
    //Serial.print("Simulated remote number : "); Serial.println(REMOTE+i, HEX);
    EEPROM.get(EEPROM_ADDRESS+2*i, code);
    //Serial.print("Current rolling code    : "); Serial.println(code);   
  }

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.setPayloadSize(4);

  //radio.printDetails();

  radio.openReadingPipe(1,addresses);
  
  // Start the radio listening for data
  radio.startListening();

  // RC Switch
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  
  
  Serial.println("20;00;Etimou RadioFrequencyLink - RFLink Gateway Clone V0.0;");
}

void loop() {
  
  
/****************** Pong Back Role ***************************/


    byte received_data[4];

    
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( received_data, sizeof(unsigned long) );         // Get the payload
      }

      Serial.print("20;");
      Serial.print(msg_counter++, HEX);
      Serial.print(";Kaku;");
      
      Serial.print("ID=");
      Serial.print(received_data[0]), HEX;
      Serial.print(";");


      Serial.print("SWITCH=");
      //Serial.print(received_data[3]);  
      Serial.print("1");  
      Serial.print(";");
      

      Serial.print("CMD=");
      if (received_data[1]) Serial.print("ON;");
      else Serial.print("OFF;");

      //Serial.print("VOLT=");
      //Serial.print(received_data[2]);
      //Serial.print(";");    

      Serial.println("");


      Serial.print("20;");
      Serial.print(msg_counter++, HEX);
      Serial.print(";Kaku;");
      
      Serial.print("ID=");
      Serial.print(received_data[0]+1), HEX;
      Serial.print(";");


      Serial.print("SWITCH=");
      //Serial.print(received_data[3]);  
      Serial.print("1");  
      Serial.print(";");
      
      Serial.print("VOLT=");
      Serial.print(received_data[2]*10, HEX);
      Serial.print(";");    

      Serial.println("");

      
   
    }

  //10;NewKaku;99CE3C;3;DOWN  
  //10;NewKaku;90151C;3;DOWN  
  //10;NewKaku;96D6B0;3;DOWN
  //10;NewKaku;93E890;3;DOWN
  
  //10;RTS;000000;1;ON

  //10;HomeEasy;71B6E3F2;2;ON
    if (Serial.available()>0){
     
      char rec = (char)Serial.read();
      if ((rec == '\n') or (rec == '\r')) rec=0;
      
      cmd[nbReadBytes]=rec;
      if ((nbReadBytes<125)&&(rec!=0)) nbReadBytes++;
      else {
        nbReadBytes =0;
        //Serial.println(cmd);
        protocol=0;
        command=0;

        char * pch;
        char index =0;
        pch = strtok (cmd,";");
        while (pch != NULL)
        {


          if (!strcmp(pch, "10")) index=1;
          //Serial.println(index, DEC);

          if (index==2){
            //Serial.println(pch);
            if (!strcmp(pch, "RTS")) protocol=PROTOCOL_SOMFY;
            else if (!strcmp(pch, "X10")) protocol=PROTOCOL_SOMFY;
            else if (!strcmp(pch, "HomeEasy")) protocol=PROTOCOL_HOMEEASY;
            else if (!strcmp(pch, "NewKaku")) protocol=PROTOCOL_NEWKAKU;
            else if (!strcmp(pch, "PING")) protocol=PROTOCOL_PING; 
          }
          if (index==3){
            address=strtol(pch, NULL, 16);
            if (protocol==PROTOCOL_HOMEEASY){
              address=address & 0x00FFFFFF;
            }

          }
          if (index==4){
            code=(int)strtol(pch, NULL, 16)-1;
         
          }
          if (index==5){
            if (!strcmp(pch, "ON")) command=ON;
            else if (!strcmp(pch, "OFF")) command=OFF;
            else if (!strcmp(pch, "UP")) command=UP;
            else if (!strcmp(pch, "DOWN")) command=DOWN;
            else if (!strcmp(pch, "STOP")) command=STOP;
            else if (!strcmp(pch, "PROG")) command=PROG;
            
          }

          pch = strtok (NULL, ";");          
          if (index >=1) index++;
        
        }
        if (index==6 || protocol==PROTOCOL_PING)
        {

          if (protocol==PROTOCOL_HOMEEASY){

            sendHomeEasy(address, code, PIN_TX, RETRIES, command==ON);
            
          }
          else if (protocol==PROTOCOL_NEWKAKU){
            
            if (address == 0xFFFFFF){
              digitalWrite(PIN_ALARM, command==ON);
            }
            else if (address == 0xFFFFFE){
              if (command == ON)
              {
                digitalWrite(PIN_ALARM, HIGH);
                delay(150);
                digitalWrite(PIN_ALARM, LOW);
                delay(300);
                digitalWrite(PIN_ALARM, HIGH);
                delay(150);
                digitalWrite(PIN_ALARM, LOW);                
              }
              else{
                digitalWrite(PIN_ALARM, HIGH);
                delay(150);
                digitalWrite(PIN_ALARM, LOW);
              }
            }
            else{
              send(address, 24, PIN_TX, RETRIES);  
            }
          }
          else if (protocol==PROTOCOL_SOMFY){
            BuildFrame(frame, command, (byte)code);
            SendCommand(frame, 2);
            for(int i = 0; i<2; i++) {
              SendCommand(frame, 7);
            }
          }
          Serial.print("20;");
          Serial.print(msg_counter++, HEX);
          Serial.print(";OK;");
          Serial.println();
        }
      }
    }
/*
 * 
 */
  if (mySwitch.available()) {
    
    unsigned long value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      return;
    } else {

      Serial.print("20;");
      Serial.print(msg_counter++, HEX);
      Serial.print(";Kaku;");
      
      Serial.print("ID=");
      Serial.print(value, HEX);
      Serial.print(";");


      Serial.print("SWITCH="); 
      Serial.print("1");  
      Serial.print(";");
      
      Serial.print("CMD=ON;");
      Serial.println("");

    }

    mySwitch.resetAvailable();
  }


} // Loop



/*HomeEasy protocol
 
Data 0 = High 275uS, Low 275uS, High 275uS, Low 1225uS
Data 1 = High 275uS, Low 1225uS, High 275uS, Low 275uS
A preamble is sent before each command which is High 275uS, Low 2675uS 
When sending a dim level a special bit is placed in bit 27

Dim bit 27 = High 275uS, Low 275uS, High 275uS, Low 275uS. This seems a bit odd, and goes agianst the manchester coding specification !
Each packet is sent 4 of 5 times with a 10mS space in between each.]
*/

void sendHomeEasyOne(int pinTx)
{
   digitalWrite(pinTx, HIGH);
   delayMicroseconds(275);
   digitalWrite(pinTx, LOW);
   delayMicroseconds(1225);
   digitalWrite(pinTx, HIGH);
   delayMicroseconds(275);
   digitalWrite(pinTx, LOW);
   delayMicroseconds(275);
}
void sendHomeEasyZero(int pinTx)
{
   digitalWrite(pinTx, HIGH);
   delayMicroseconds(275);
   digitalWrite(pinTx, LOW);
   delayMicroseconds(275);
   digitalWrite(pinTx, HIGH);
   delayMicroseconds(275);
   digitalWrite(pinTx, LOW);
   delayMicroseconds(1225);
}



void sendHomeEasy(unsigned long address, int code, int pinTx, int nbOfRetries, bool state)
{
  for (int n=0; n<nbOfRetries; n++)
  {
    digitalWrite(pinTx, HIGH);
    delayMicroseconds(275);
    digitalWrite(pinTx, LOW);
    delayMicroseconds(2675);  
  
    //address 
    for (int i = 25; i >= 0; i--) {
      if (address & (1L << i)) sendHomeEasyOne(pinTx);
      else sendHomeEasyZero(pinTx);

    }
    //group flag, not managed
    sendHomeEasyZero(pinTx);

    //command 0 or 1
    if (state) sendHomeEasyOne(pinTx);
    else sendHomeEasyZero(pinTx); 

    //device code
    for (int i = 3; i >= 0; i--) {
      if (code & (1L << i)) sendHomeEasyOne(pinTx);
      else sendHomeEasyZero(pinTx);

    }
    //end
   digitalWrite(pinTx, HIGH);
   delayMicroseconds(275);
   digitalWrite(pinTx, LOW);
     
    delay(10);
    }

    
}

void send(unsigned long data, int dataLength, int pinTx, int nbOfRetries)
{
  for (int n=0; n<nbOfRetries; n++)
  {
    digitalWrite(pinTx, HIGH);
    delayMicroseconds(2000);
    digitalWrite(pinTx, LOW);
    delayMicroseconds(2000);  
  
  
    for (int i = dataLength-1; i >= 0; i--) {
      if (data & (1L << i))
      {
        digitalWrite(pinTx, HIGH);
        delayMicroseconds(1020);
        digitalWrite(pinTx, LOW);
        delayMicroseconds(510);
      } 
      else
      {
        digitalWrite(pinTx, HIGH);
        delayMicroseconds(510);
        digitalWrite(pinTx, LOW);
        delayMicroseconds(1020);
      }
    }
  }
}
void BuildFrame(byte *frame, byte button, byte remoteNumber) {
  unsigned int Code;
  EEPROM.get(EEPROM_ADDRESS+2*remoteNumber, Code);
  frame[0] = 0xA7; // Encryption key. Doesn't matter much
  frame[1] = button << 4;  // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = Code >> 8;    // Rolling code (big endian)
  frame[3] = Code;         // Rolling code
  frame[4] = REMOTE+remoteNumber >> 16; // Remote address
  frame[5] = REMOTE+remoteNumber >>  8; // Remote address
  frame[6] = REMOTE+remoteNumber;       // Remote address

  //Serial.print("Frame         : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      //Serial.print("0");     // nibble is a 0.
    }
    //Serial.print(frame[i],HEX); Serial.print(" ");
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

  //Serial.println(""); Serial.print("With checksum : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      //Serial.print("0");
    }
    //Serial.print(frame[i],HEX); Serial.print(" ");
  }

  
// Obfuscation: a XOR of all the bytes
  for(byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i-1];
  }

  //Serial.println(""); Serial.print("Obfuscated    : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      //Serial.print("0");
    }
    //Serial.print(frame[i],HEX); Serial.print(" ");
  }
  //Serial.println("");
  //Serial.print("Rolling Code  : "); Serial.println(code);
  EEPROM.put(EEPROM_ADDRESS+2*remoteNumber, Code + 1); //  We store the value of the rolling code in the
                                        // EEPROM. It should take up to 2 adresses but the
                                        // Arduino function takes care of it.
}

void SendCommand(byte *frame, byte sync) {
  if(sync == 2) { // Only with the first frame.
  //Wake-up pulse & Silence
    //PORTD |= 1<<PORT_TX;
    digitalWrite(PIN_TXS, HIGH);
    delayMicroseconds(9415);
    //PORTD &= !(1<<PORT_TX);
    digitalWrite(PIN_TXS, LOW);
    delayMicroseconds(89565);
  }

// Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    //PORTD |= 1<<PORT_TX;
    digitalWrite(PIN_TXS, HIGH);
    delayMicroseconds(4*SYMBOL);
    //PORTD &= !(1<<PORT_TX);
    digitalWrite(PIN_TXS, LOW);
    delayMicroseconds(4*SYMBOL);
  }

// Software sync
  //PORTD |= 1<<PORT_TX;
  digitalWrite(PIN_TXS, HIGH);
  delayMicroseconds(4550);
  //PORTD &= !(1<<PORT_TX);
  digitalWrite(PIN_TXS, LOW);
  delayMicroseconds(SYMBOL);
  
  
//Data: bits are sent one by one, starting with the MSB.
  for(byte i = 0; i < 56; i++) {
    if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
      //PORTD &= !(1<<PORT_TX);
      digitalWrite(PIN_TXS, LOW);
      delayMicroseconds(SYMBOL);
      //PORTD ^= 1<<5;
      digitalWrite(PIN_TXS, HIGH);
      delayMicroseconds(SYMBOL);
    }
    else {
      //PORTD |= (1<<PORT_TX);
      digitalWrite(PIN_TXS, HIGH);
      delayMicroseconds(SYMBOL);
      //PORTD ^= 1<<5;
      digitalWrite(PIN_TXS, LOW);
      delayMicroseconds(SYMBOL);
    }
  }
  
  //PORTD &= !(1<<PORT_TX);
  digitalWrite(PIN_TXS, LOW);
  delayMicroseconds(30415); // Inter-frame silence
}

