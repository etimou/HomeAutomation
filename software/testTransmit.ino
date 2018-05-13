#define pin 5
#define retries 5

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  send(10079804, 24, pin, retries);
  delay(5000);
  send(9442588, 24, pin, retries);
  delay(5000);
//9885360 9693328

/* code on the remote
 *  button 1: code 0
 *  button 2: code 1
 *  button 3: code 2
 *  button 4: code 3
 *  button G: code 0+ group flag
 *  
 */
  //sendHomeEasy(11985906, 0, pin, retries, 1);
 

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

