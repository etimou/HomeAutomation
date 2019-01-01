#include "MyHwAVR.h"
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>

//#define DOOR_SENSOR
//#define MOTION_SENSOR

#define PRIMARY_BUTTON_PIN 2   // Arduino Digital I/O pin for button/reed switch
#define SECONDARY_BUTTON_PIN 3 // Arduino Digital I/O pin for button/reed switch
#define LED_PIN 13

#define SLEEP_TIME 3600000L


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 8 */
RF24 radio(9,8);
byte addresses[6] = "1Node";
/**********************************************************/


bool s=0;
byte voltage=0;
byte sensorID=0;

unsigned long calib=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  
  // Setup the buttons
  pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

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

  delay(100);
  
  //radio setup
  radioSetup();

  
  Serial.println("Initialisation complete."); delay(100);

}




void loop() {
  // put your main code here, to run repeatedly:

  delay(100);
  s= digitalRead(PRIMARY_BUTTON_PIN);

  //Serial.println(s);
  voltage = readVcc();

  sendRadioData();
  
  if (digitalRead(PRIMARY_BUTTON_PIN)==s){//go to sleep if status hasn't changed during data transmission
    hwSleep(0, CHANGE, 1, CHANGE, SLEEP_TIME);
  }
}

void sendRadioData(){
    //Power up the radio
    radio.powerUp();
  
    Serial.print(F("Now sending"));

    
    byte data_to_send[4];
    data_to_send[0]= sensorID;
    data_to_send[1]= (byte)s;
    data_to_send[2]= voltage;
    data_to_send[3]= 0xAA ;

    
    Serial.print("...");
    Serial.print(data_to_send[0], HEX);
    Serial.print(" ");
    Serial.print(data_to_send[1], HEX);
    Serial.print(" ");    
    Serial.print(data_to_send[2], HEX);
    Serial.print(" ");
    Serial.print(data_to_send[3], HEX);
    Serial.println("%");

    
     if (!radio.write( data_to_send, 4) ){
       Serial.println("failed");
     }

    //Power off the radio    
    radio.powerDown();
  
}


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
  //result = 1108835L / result; // calibration for sensor proto wired
  //result = 1151826L / result; // calibration for 1st, PCB proto
  
  //from millivolt to percentage
  result=map(result, 3000, 3400, 0, 100);//adjust 2nd value to get 80% at 3.3VDC connected to regulator 2nd proto

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

