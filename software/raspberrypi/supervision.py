#!/usr/bin/python

import select, socket, sys, Queue
import serial
import datetime
import os
import time
import sqlite3
import logging
from logging.handlers import RotatingFileHandler

# TCP stuff -----------------------------------------------

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#server.setblocking(0)
server.bind(('localhost', 50000))
server.listen(5)
ser = serial.Serial('/dev/ttyUSB0', 57600, timeout=2)
inputs = [server]
inputs.append(ser)
outputs = []
message_queues = {}
# ------------------------------------------------------------


# Logging stuff ----------------------------------------------
# create logger
logger = logging.getLogger('simple_example')
logger.setLevel(logging.DEBUG)

# create console handler and set level to debug
ch = RotatingFileHandler('/home/pi/HomeAutomation/software/raspberrypi/logging.log', maxBytes=65536, backupCount=1)
ch.setLevel(logging.DEBUG)

# create formatter
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

# add formatter to ch
ch.setFormatter(formatter)

# add ch to logger
logger.addHandler(ch)
# ------------------------------------------------------------


# Connection to database -------------------------------------
conn = sqlite3.connect('/home/pi/HomeAutomation/software/raspberrypi/MyHouse.db')
conn.row_factory = sqlite3.Row
c = conn.cursor()

db = c.execute('SELECT * FROM sensors ORDER BY ID')
for row in db:
	print row
# ------------------------------------------------------------

# Recover alarm status from database -------------------------
db = c.execute('select * from alarm')
[alarmActivationStatus, alarmIntrusionStatus, alarmAddressOnHome, alarmAddressOnAway, alarmAddressOff] = db.fetchone()

#print "Alarm activation status : "+  alarmActivationStatus
logger.info("Alarm activation status at startup : "+  alarmActivationStatus)

#____________________________________________________________________________________

def processAlarmActivation(address):
	# Manage alarm activation
	global alarmActivationStatus, alarmIntrusionStatus, alarmAddressOnHome, alarmAddressOnAway, alarmAddressOff
	if (address == alarmAddressOnHome or address == "ON_HOME"):
		#print ("Setting Alarm ON HOME")
	  	logger.info("Setting Alarm ON HOME")
		c.execute('UPDATE alarm SET activationStatus = "ON_HOME"')
		alarmActivationStatus = "ON_HOME"
		ser.write("10;NewKaku;FFFFFE;1;ON\r\n")
	elif (address == alarmAddressOnAway or address == "ON_AWAY"):
		#print ("Setting Alarm ON AWAY")
	  	logger.info("Setting Alarm ON AWAY")
		c.execute('UPDATE alarm SET activationStatus = "ON_AWAY"')
		alarmActivationStatus = "ON_AWAY"
		ser.write("10;NewKaku;FFFFFE;1;ON\r\n")
	elif (address == alarmAddressOff or address == "OFF"):
		#print ("Setting Alarm OFF")
	  	logger.info("Setting Alarm OFF")
		c.execute('UPDATE alarm SET activationStatus = "OFF"')
		c.execute('UPDATE alarm SET intrusionStatus = "NO"')
		alarmActivationStatus = "OFF"
		alarmIntrusionStatus = "NO"
		ser.write("10;NewKaku;FFFFFE;1;OFF\r\n")
		ser.write("10;NewKaku;FFFFFF;1;OFF\r\n")
	conn.commit()
	return 1

def processCommandFromSerial(data):
	print data
	global alarmActivationStatus, alarmIntrusionStatus, alarmAddressOnHome, alarmAddressOnAway, alarmAddressOff
  	command = data.split(";")
  	if len(command)<5:
		return 0

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
		return 0

	# get the rest of the message
	status = command[5][4:].upper()
	time = str(datetime.datetime.now())[:-3]

	# more info on the selected sensor
	[Type, Name] = [db["type"], db["Name"]]
	if Type=="Simple433":
		status=None
  
	# update the database with date and status
	sql = 'UPDATE sensors SET LastSeen ="'+ time + '"'+ ', Status="'+ str(status) + '" WHERE address = ' + str(address)
	#print sql
	logger.info("Sensor "+ Name + " status " + str(status) )
	c.execute(sql)

	# Manage alarm activation
	processAlarmActivation(address)



	if (status in [None, "ON"]) and ((alarmActivationStatus == "ON_HOME" and db["InAlarmHome"]) or (alarmActivationStatus == "ON_AWAY" and db["InAlarmAway"])):
	  	#print "intrusion detectee"
	  	logger.info("intrusion detectee")
		c.execute('UPDATE alarm SET intrusionStatus = "YES"')
	  	alarmIntrusionStatus = "YES"
		ser.write("10;NewKaku;FFFFFF;1;ON\r\n")

		conn.commit()
	return 1




def processCommandFromTCP(data):
	if data in ["ON_HOME","ON_AWAY","OFF"]:
		processAlarmActivation(data)
	else:
		logger.info("Activation " + data)		
		ser.write(data+'\n')


while inputs:
    readable, writable, exceptional = select.select(
        inputs, outputs, inputs)
    for s in readable:
        if s is server:
            connection, client_address = s.accept()
            connection.setblocking(0)
            inputs.append(connection)
            message_queues[connection] = Queue.Queue()
	elif s is ser:
	    data=ser.readline()
	    processCommandFromSerial(data)
        else:
            #data = s.recv(1024)
	    data = s.makefile().readline().rstrip()
            if data:
                message_queues[s].put(data)
                if s not in outputs:
                    outputs.append(s)
		processCommandFromTCP(data)
            else:
                if s in outputs:
                    outputs.remove(s)
                inputs.remove(s)
                s.close()
                del message_queues[s]

    for s in writable:
        try:
            next_msg = message_queues[s].get_nowait()
        except Queue.Empty:
            outputs.remove(s)
        else:
            s.send(next_msg)

    for s in exceptional:
        inputs.remove(s)
        if s in outputs:
            outputs.remove(s)
        s.close()
        del message_queues[s]





