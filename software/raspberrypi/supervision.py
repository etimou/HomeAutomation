#!/usr/bin/python

import serial
import datetime
import os
import time
import sqlite3

conn = sqlite3.connect('/home/eti/HomeAutomation/software/raspberrypi/MyHouse.db')
conn.row_factory = sqlite3.Row
c = conn.cursor()

db = c.execute('SELECT * FROM sensors ORDER BY ID')
for row in db:
	print row

#____________________________________________________________________________________

db = c.execute('select * from alarm')
[alarmActivationStatus, alarmIntrusionStatus, alarmAddressOnHome, alarmAddressOnAway, alarmAddressOff] = db.fetchone()

print "Alarm activation status : "+  alarmActivationStatus


#____________________________________________________________________________________


ser = serial.Serial('/dev/ttyUSB0', 57600)

while (1):
  read=ser.readline()
  command = read.split(";")
  if len(command)<4:
	continue
  address = command[3][3:]
  address = int(address, 16)
  time = str(datetime.datetime.now())[:-3]
  sql = 'UPDATE sensors SET LastSeen ="'+ time + '" WHERE address = ' + str(address)
  print sql
  c.execute(sql)

  if (address == alarmAddressOnHome):
	print ("Setting Alarm ON HOME")
	c.execute('UPDATE alarm SET activationStatus = "ON HOME"')
	alarmActivationStatus = "ON HOME"
  elif (address == alarmAddressOnAway):
	print ("Setting Alarm ON AWAY")
	c.execute('UPDATE alarm SET activationStatus = "ON AWAY"')
	alarmActivationStatus = "ON AWAY"
  elif (address == alarmAddressOff):
	print ("Setting Alarm OFF")
	c.execute('UPDATE alarm SET activationStatus = "OFF"')
	alarmActivationStatus = "OFF"
	alarmIntrusionStatus = "NO"
  
  sql ='SELECT * FROM sensors WHERE address = ' + str(address)
  db = c.execute(sql)
  db = db.fetchone()
  if (alarmActivationStatus == "ON HOME" and db["InAlarmHome"]) or (alarmActivationStatus == "ON AWAY" and db["InAlarmAway"]):
  	print "intrusion detectee"
  	alarmIntrusionStatus = "YES"

  conn.commit()





