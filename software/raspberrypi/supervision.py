#!/usr/bin/python

import serial
import datetime
import os
import time
import sqlite3

conn = sqlite3.connect('/home/eti/MyHouse.db')
c = conn.cursor()

db = c.execute('SELECT * FROM sensors ORDER BY ID')
for row in db:
	print row




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
  conn.commit()

