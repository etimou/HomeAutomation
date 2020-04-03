#include <EEPROM.h>
#include <LowPower.h>

void setup() {
  // put your setup code here, to run once:



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
  Serial.begin(57600);

  LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);

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
    return 0;
  }
  
  
  unsigned long calib = long(1125300L * float(3320) / result); //write in float(...) the value you measure with a multimeter

  Serial.print("Storing calibration value in EEPROM: ");
  Serial.println(calib);
  EEPROM.put(1,calib);

  byte sensorID = 12;
  Serial.print("Storing sensor ID value in EEPROM: ");
  Serial.println(sensorID);
  EEPROM.write(0, sensorID);
;


  

}

void loop() {
  // put your main code here, to run repeatedly:

}
