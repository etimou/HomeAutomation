#!/usr/bin/python

import serial
import datetime
import os
import time
import sqlite3
import socket

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

print "Alarm activation status : "+  alarmActivationStatus


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

  address = command[3][3:]
  address = int(address, 16)
  time = str(datetime.datetime.now())[:-3]
  sql = 'UPDATE sensors SET LastSeen ="'+ time + '" WHERE address = ' + str(address)
  print sql
  c.execute(sql)



  if (address == alarmAddressOnHome):
	print ("Setting Alarm ON HOME")
	c.execute('UPDATE alarm SET activationStatus = "ON_HOME"')
	alarmActivationStatus = "ON_HOME"
	ser.write("10;NewKaku;FFFFFE;1;ON\r\n")
  elif (address == alarmAddressOnAway):
	print ("Setting Alarm ON AWAY")
	c.execute('UPDATE alarm SET activationStatus = "ON_AWAY"')
	alarmActivationStatus = "ON_AWAY"
	ser.write("10;NewKaku;FFFFFE;1;ON\r\n")


  elif (address == alarmAddressOff):
	print ("Setting Alarm OFF")
	c.execute('UPDATE alarm SET activationStatus = "OFF"')
	c.execute('UPDATE alarm SET intrusionStatus = "NO"')
	alarmActivationStatus = "OFF"
	alarmIntrusionStatus = "NO"
	ser.write("10;NewKaku;FFFFFE;1;OFF\r\n")
	ser.write("10;NewKaku;FFFFFF;1;OFF\r\n")




  
  sql ='SELECT * FROM sensors WHERE address = ' + str(address)
  db = c.execute(sql)
  db = db.fetchone()
  if (alarmActivationStatus == "ON_HOME" and db["InAlarmHome"]) or (alarmActivationStatus == "ON_AWAY" and db["InAlarmAway"]):
  	print "intrusion detectee"
	c.execute('UPDATE alarm SET intrusionStatus = "YES"')
  	alarmIntrusionStatus = "YES"
	ser.write("10;NewKaku;FFFFFF;1;ON\r\n")

  conn.commit()





