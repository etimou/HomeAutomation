#define MY_CORE_ONLY

#include <MySensors.h> // for the sleep function only
#include <SPI.h>
#include "RF24.h"

//#define DOOR_SENSOR
#define MOTION_SENSOR

#define SLEEP_TIME 3600000L


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 8 */
RF24 radio(9,8);
/**********************************************************/

byte addresses[6] = "1Node";
//byte sensorID = 10;// first prototype
//byte sensorID = 11;// second prototype
byte sensorID = 12;//third prototype



#define PRIMARY_BUTTON_PIN 2   // Arduino Digital I/O pin for button/reed switch
#define SECONDARY_BUTTON_PIN 3 // Arduino Digital I/O pin for button/reed switch
#define LED_PIN 13

#if (PRIMARY_BUTTON_PIN < 2 || PRIMARY_BUTTON_PIN > 3)
#error PRIMARY_BUTTON_PIN must be either 2 or 3 for interrupts to work
#endif
#if (SECONDARY_BUTTON_PIN < 2 || SECONDARY_BUTTON_PIN > 3)
#error SECONDARY_BUTTON_PIN must be either 2 or 3 for interrupts to work
#endif
#if (PRIMARY_BUTTON_PIN == SECONDARY_BUTTON_PIN)
#error PRIMARY_BUTTON_PIN and BUTTON_PIN2 cannot be the same
#endif

//#define SECONDARY_BUTTON_IS_USED


void setup()
{
	// Setup the buttons
	pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP);
	pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Initialising..."); delay(100);
  
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  
   //radio setup
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
  
  Serial.println("Initialisation complete."); delay(100);
  
}

void presentation()
{
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
	uint8_t value;

	// Short delay to allow buttons to properly settle
	sleep(5);

  //Read and sent the Button 1 status
	value = digitalRead(PRIMARY_BUTTON_PIN);
	
  //Read and sent the Button 2 status
#ifdef SECONDARY_BUTTON_IS_USED
	value = digitalRead(SECONDARY_BUTTON_PIN);
#endif
  //////////////////////////////////////////////////////
  /////////////////////////////////////////////////////
  // Put here the code to be executed when the sensor is awaken
    //Power up the radio
    radio.powerUp();
  
    Serial.print(F("Now sending"));

    //unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
    byte voltage = readVcc();
    
    byte data_to_send[4];
    data_to_send[0]= sensorID;
    data_to_send[1]= (byte)value;
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
       Serial.println(F("failed"));
     }
        
    radio.powerDown();



  /////////////////////////////////////////////////////////
 /////////////////////////////////////////////////////////
  if (digitalRead(PRIMARY_BUTTON_PIN)==value)
  {
    #if defined(MOTION_SENSOR)
    if (value!=0)
    {
      //go to sleep first, with no possibility to wake up
      sleep(180000L); 
    }
    #endif
    
    // Sleep until something happens with the sensor
    sleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, SLEEP_TIME);
    
  }
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

  delay(15); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000 //ok for proto w/ intermediate PCB
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



