// Wake from deep sleep with a keypress demonstration

// Author: Nick Gammon
// Date: 18th November 2012

#include "Keypad2.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>

const byte ROWS = 4;
const byte COLS = 3; 

char keys[ROWS][COLS] = 
  {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'},
  };

byte rowPins[ROWS] = {6, 7, 8, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4}; //connect to the column pinouts of the keypad
bool state;

/*
arduino keypad 
6 2
7 7
8 6
9 4

2 3
3 1
4 5
*/


/*                           NRF24L01      
 * NC----------------------IRQ 8 7 MISO---------------12
 * 11---------------------MOSI 6 5 SCK----------------13
 *  5-----------------------CS 4 3 CE-----------------10
 *VCC----------------------VCC 2 1 GND----------------GND
 */

char secretCodeON[4] = {'7', '5', '1', '#'};
char secretCodeOFF[4] = {'7', '5', '0', '#'};
char enteredCode[4] = {'0', '0', '0', '0'};
  
// number of items in an array
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

const byte ledPin = A0;
unsigned long calib=0;
byte sensorID=0;
byte voltage=0;

  // Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


/* Set up nRF24L01 radio on SPI bus plus pins 10 & 9 */
RF24 radio(10,5);
byte addresses[6] = "1Node";

/*

Pin change interrupts.

Pin                  Mask / Flag / Enable

D0    PCINT16 (PCMSK2 / PCIF2 / PCIE2)
D1    PCINT17 (PCMSK2 / PCIF2 / PCIE2)
D2    PCINT18 (PCMSK2 / PCIF2 / PCIE2)
D3    PCINT19 (PCMSK2 / PCIF2 / PCIE2)
D4    PCINT20 (PCMSK2 / PCIF2 / PCIE2)
D5    PCINT21 (PCMSK2 / PCIF2 / PCIE2)
D6    PCINT22 (PCMSK2 / PCIF2 / PCIE2)
D7    PCINT23 (PCMSK2 / PCIF2 / PCIE2)
D8    PCINT0 (PCMSK0 / PCIF0 / PCIE0)
D9    PCINT1 (PCMSK0 / PCIF0 / PCIE0)
D10   PCINT2 (PCMSK0 / PCIF0 / PCIE0)
D11   PCINT3 (PCMSK0 / PCIF0 / PCIE0)
D12   PCINT4 (PCMSK0 / PCIF0 / PCIE0)
D13   PCINT5 (PCMSK0 / PCIF0 / PCIE0)
A0    PCINT8 (PCMSK1 / PCIF1 / PCIE1)
A1    PCINT9 (PCMSK1 / PCIF1 / PCIE1)
A2    PCINT10 (PCMSK1 / PCIF1 / PCIE1)
A3    PCINT11 (PCMSK1 / PCIF1 / PCIE1)
A4    PCINT12 (PCMSK1 / PCIF1 / PCIE1)
A5    PCINT13 (PCMSK1 / PCIF1 / PCIE1)

*/

// turn off interrupts until we are ready
ISR (PCINT0_vect)
  {
  PCICR = 0;  // cancel pin change interrupts
  } // end of ISR (PCINT0_vect)

ISR (PCINT1_vect)
  {
  PCICR = 0;  // cancel pin change interrupts
  } // end of ISR (PCINT1_vect)

ISR (PCINT2_vect)
  {
  PCICR = 0;  // cancel pin change interrupts
  } // end of ISR (PCINT2_vect)

byte readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(70); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = calib / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000 //ok for proto w/ intermediate PCB
  
  //from millivolt to percentage
  result=map(result, 3000, 3400, 0, 100); //3.0V->0% 3.4V->100%

  if (result >100) return 100;
  if (result <0) return 0;
  return (byte)result; // Vcc in millivolts

/*
While the large tolerance of the internal 1.1 volt reference greatly limits the accuracy of 
this measurement, for individual projects we can compensate for greater accuracy. To do so, 
simply measure your Vcc with a voltmeter and with our readVcc() function. Then, replace the 
constant 1125300L with a new constant:
scale_constant = internal1.1Ref * 1023 * 1000
where
internal1.1Ref = 1.1 * Vcc1 (per voltmeter) / Vcc2 (per readVcc() function)
This calibrated value will be good for the AVR chip measured only, and may be subject to temperature variation. Feel free to experiment with your own measurements.
*/
}

void radioSetup(){
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  //radio.setPALevel(RF24_PA_LOW);
  
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  //radio.setRetries(5,15);
  radio.setPayloadSize(4);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses);
  radio.stopListening();
  radio.powerDown();
}
void sendRadioData(){
    //Power up the radio
    radio.powerUp();
  
    byte data_to_send[4];
    data_to_send[0]= sensorID;
    data_to_send[1]= state;
    data_to_send[2]= voltage;
    data_to_send[3]= 0;

    radio.write( data_to_send, 4);

    //Power off the radio    
    radio.powerDown();
  
}

void setup ()
{
  pinMode (ledPin, OUTPUT);

  //radio setup
  radioSetup();

  // pin change interrupt masks (see above list)
  PCMSK2 |= bit (PCINT22);   // pin 6
  PCMSK2 |= bit (PCINT23);   // pin 7
  PCMSK0 |= bit (PCINT0);    // pin 8
  PCMSK0 |= bit (PCINT1);    // pin 9



  Serial.begin(57600);
  // get the device ID from EEPROM address 0
  //EEPROM.write(0, 12);
  sensorID = EEPROM.read(0);
  Serial.print("Device ID=");
  Serial.println(sensorID);

  //get the calibration value
  //EEPROM.put(1,1108835L);
  EEPROM.get(1, calib);
  Serial.print("Calibration value=");
  Serial.println(calib);

  Serial.println("Initialisation complete.");

  // calibration and Id
  char readchar[5];
  char nchar=0;
  while (millis()<5000){
    while (Serial.available()>0){
      if (nchar<=4){
        readchar[nchar]=Serial.read();
        nchar++;
      }
      if (nchar==5){
        if (readchar[0]=='s' && readchar[3]=='c'){
          char newID = (readchar[1]-48)*10+readchar[2]-48;
          if (newID){
            saveIdToEEPROM((readchar[1]-48)*10+readchar[2]-48);
          }
          if ((readchar[4]-48)==1){
            calibrationVoltage(3300);
          }
        }
        else{
          Serial.print("Wrong command...");
        }
        Serial.println("Starting main program");

        delay(50);
        while (Serial.available()>0){Serial.read();}
        
        return;
      }
    } 
  }
  Serial.println("No command received... Starting main program");
  Serial.end();

  delay(100);


}  // end of setup

// set pins as keypad library expects them
// or call: kpd.initializePins ();
//    however in the library I have that is a private method

void reconfigurePins ()
  {
  byte i;
  
  // go back to all pins as per the keypad library
  
  for (i = 0; i < NUMITEMS (colPins); i++)
    {
    pinMode (colPins [i], OUTPUT);
    digitalWrite (colPins [i], HIGH); 
    }  // end of for each column 

  for (i = 0; i < NUMITEMS (rowPins); i++)
    {
    pinMode (rowPins [i], INPUT_PULLUP);
    }   // end of for each row

  }  // end of reconfigurePins

void goToSleep ()
  {
  byte i;
   
  // set up to detect a keypress
  for (i = 0; i < NUMITEMS (colPins); i++)
    {
    pinMode (colPins [i], OUTPUT);
    digitalWrite (colPins [i], LOW);   // columns low
    }  // end of for each column

  for (i = 0; i < NUMITEMS (rowPins); i++)
    {
    pinMode (rowPins [i], INPUT_PULLUP);
    }  // end of for each row
    
   // now check no pins pressed (otherwise we wake on a key release)
   for (i = 0; i < NUMITEMS (rowPins); i++)
    {
    if (digitalRead (rowPins [i]) == LOW)
       {
       reconfigurePins ();
       return; 
       } // end of a pin pressed
    }  // end of for each row
  
  // overcome any debounce delays built into the keypad library
  delay (50);
  
  // at this point, pressing a key should connect the high in the row to the 
  // to the low in the column and trigger a pin change
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();

  byte old_ADCSRA = ADCSRA;
  // disable ADC to save power
  ADCSRA = 0;  

  power_all_disable ();  // turn off various modules
   
  PCIFR  |= bit (PCIF0) | bit (PCIF1) | bit (PCIF2);   // clear any outstanding interrupts
  PCICR  |= bit (PCIE0) | bit (PCIE1) | bit (PCIE2);   // enable pin change interrupts
   
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  sleep_cpu ();  
 
  // cancel sleep as a precaution
  sleep_disable();
  power_all_enable ();   // enable modules again
  ADCSRA = old_ADCSRA;   // re-enable ADC conversion
  
  // put keypad pins back how they are expected to be
  reconfigurePins ();
    
  }  // end of goToSleep
  
void loop () 
  {
   
   byte key =  kpd.getKey();
   if (!key)
     {
     // no key pressed? go to sleep
     goToSleep ();
     return;
     }


  // confirmation we woke - flash LED number of times
  // for the appropriate pin (eg. pin 1: one time) 
  /*
  for (byte i = 0; i < (key - '0'); i++)
    {
    digitalWrite (ledPin, HIGH);
    delay (500); 
    digitalWrite (ledPin, LOW);
    delay (500); 
    } // end of for loop
  */
  digitalWrite (ledPin, HIGH);
  delay (300); 
  digitalWrite (ledPin, LOW);
  
  enteredCode[0]= enteredCode[1];
  enteredCode[1]= enteredCode[2];
  enteredCode[2]= enteredCode[3];
  enteredCode[3] = key;

  if ((enteredCode[0]==secretCodeON[0])&&(enteredCode[1]==secretCodeON[1])&&(enteredCode[2]==secretCodeON[2])&&(enteredCode[3]==secretCodeON[3]))
  {
    state = 1;
    voltage = readVcc();
    
    sendRadioData();
    
    for (byte i = 0; i < 10; i++)
    {
      digitalWrite (ledPin, HIGH);
      delay (100); 
      digitalWrite (ledPin, LOW);
      delay (100); 
    }


      
  }
  if ((enteredCode[0]==secretCodeOFF[0])&&(enteredCode[1]==secretCodeOFF[1])&&(enteredCode[2]==secretCodeOFF[2])&&(enteredCode[3]==secretCodeOFF[3]))
  {
    state = 0;
    voltage = readVcc();
    
    sendRadioData();
    
    digitalWrite (ledPin, HIGH);
    delay (3000); 
    digitalWrite (ledPin, LOW);
    
  }
  
 } // end of loop


 void saveIdToEEPROM(byte IdValue){
  sensorID = IdValue;
  Serial.print("Storing sensor ID value in EEPROM: ");
  Serial.println(IdValue);
  EEPROM.write(0, IdValue);
}
void calibrationVoltage(int milliVolt){
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(70); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000

  Serial.print("Measured ");
  Serial.print(result/1000.0);
  Serial.println("V");


  if ((result<3000) || (result>4000)){
    Serial.println("Voltage not in acceptable range for 3.3V calibration");
    return;
  }
  
  
  calib = (unsigned long)(1125300L * float(milliVolt) / result); //write in float(...) the value you measure with a multimeter

  Serial.print("Storing calibration value in EEPROM: ");
  Serial.println(calib);
  EEPROM.put(1,calib);
}
