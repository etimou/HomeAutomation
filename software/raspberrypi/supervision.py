#!/usr/bin/python

import serial
import datetime
import os
import time
import sqlite3
import logging
from logging.handlers import RotatingFileHandler

# create logger
logger = logging.getLogger('simple_example')
logger.setLevel(logging.DEBUG)

# create console handler and set level to debug
ch = RotatingFileHandler('/home/pi/HomeAutomation/software/raspberrypi/logging.log', maxBytes=1024)
ch.setLevel(logging.DEBUG)

# create formatter
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

# add formatter to ch
ch.setFormatter(formatter)

# add ch to logger
logger.addHandler(ch)



conn = sqlite3.connect('/home/pi/HomeAutomation/software/raspberrypi/MyHouse.db')
conn.row_factory = sqlite3.Row
c = conn.cursor()

db = c.execute('SELECT * FROM sensors ORDER BY ID')
for row in db:
	print row





#____________________________________________________________________________________
#
# Recover alarm status from database
db = c.execute('select * from alarm')
[alarmActivationStatus, alarmIntrusionStatus, alarmAddressOnHome, alarmAddressOnAway, alarmAddressOff] = db.fetchone()

#print "Alarm activation status : "+  alarmActivationStatus
logger.info("Alarm activation status at startup : "+  alarmActivationStatus)

#____________________________________________________________________________________



ser = serial.Serial('/dev/ttyUSB0', 57600, timeout=2)

while (1):

  #recover the alarm activation status from db in case it was modified externally
  alarmActivationStatus = c.execute('select * from alarm').fetchone()["ActivationStatus"]

  # read serial port, loop again if empty
  read=ser.readline()
  command = read.split(";")
  if len(command)<5:
	continue

  # get the address
  address = command[3][3:]
  address = int(address, 16)

  # get sensor info from database
  sql ='SELECT * FROM sensors WHERE address = ' + str(address)
  db = c.execute(sql)
  db = db.fetchone()

  if db==None:
  	#print "Received message from unknown address " , address
  	logger.info("Received message from unknown address " + str(address))
  	continue

  # get the rest of the message
  status = command[5][4:].upper()
  time = str(datetime.datetime.now())[:-3]

  type = db["type"]
  if type=="Simple433":
  	status=None
  
  # update the database with date and status
  sql = 'UPDATE sensors SET LastSeen ="'+ time + '"'+ ', Status="'+ str(status) + '" WHERE address = ' + str(address)
  #print sql
  logger.info("Sensor "+ str(address)+ " status " + str(status) )
  c.execute(sql)



  if (address == alarmAddressOnHome):
	#print ("Setting Alarm ON HOME")
  	logger.info("Setting Alarm ON HOME")
	c.execute('UPDATE alarm SET activationStatus = "ON_HOME"')
	alarmActivationStatus = "ON_HOME"
	ser.write("10;NewKaku;FFFFFE;1;ON\r\n")
  elif (address == alarmAddressOnAway):
	#print ("Setting Alarm ON AWAY")
  	logger.info("Setting Alarm ON AWAY")
	c.execute('UPDATE alarm SET activationStatus = "ON_AWAY"')
	alarmActivationStatus = "ON_AWAY"
	ser.write("10;NewKaku;FFFFFE;1;ON\r\n")


  elif (address == alarmAddressOff):
	#print ("Setting Alarm OFF")
  	logger.info("Setting Alarm OFF")
	c.execute('UPDATE alarm SET activationStatus = "OFF"')
	c.execute('UPDATE alarm SET intrusionStatus = "NO"')
	alarmActivationStatus = "OFF"
	alarmIntrusionStatus = "NO"
	ser.write("10;NewKaku;FFFFFE;1;OFF\r\n")
	ser.write("10;NewKaku;FFFFFF;1;OFF\r\n")



  if (status in [None, "ON"]) and ((alarmActivationStatus == "ON_HOME" and db["InAlarmHome"]) or (alarmActivationStatus == "ON_AWAY" and db["InAlarmAway"])):
  	#print "intrusion detectee"
  	logger.info("intrusion detectee")
	c.execute('UPDATE alarm SET intrusionStatus = "YES"')
  	alarmIntrusionStatus = "YES"
	ser.write("10;NewKaku;FFFFFF;1;ON\r\n")

  conn.commit()





