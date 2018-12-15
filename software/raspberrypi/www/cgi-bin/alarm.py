#! /usr/bin/python
#
from topbar import printTopBar
import sqlite3
import os
import logging
from logging.handlers import RotatingFileHandler

# create logger
logger = logging.getLogger('simple_example')
logger.setLevel(logging.DEBUG)

# create console handler and set level to debug
ch = RotatingFileHandler('/home/pi/HomeAutomation/software/raspberrypi/logging2.log', maxBytes=1024)
ch.setLevel(logging.DEBUG)

# create formatter
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

# add formatter to ch
ch.setFormatter(formatter)

# add ch to logger
logger.addHandler(ch)




command = os.environ['QUERY_STRING'][8:]



conn = sqlite3.connect('/home/pi/HomeAutomation/software/raspberrypi/MyHouse.db')
conn.row_factory = sqlite3.Row
c = conn.cursor()


if command in ["ON_AWAY","ON_HOME","OFF"]:
	sql = 'UPDATE alarm SET activationStatus = "' + command +'"'
	c.execute(sql)
	if command == "OFF":
		c.execute('UPDATE alarm SET intrusionStatus = "NO"')
	conn.commit()
	
	if command == "OFF":
		os.system('echo "10;NewKaku;FFFFFE;1;OFF" > /dev/ttyUSB0')
		
	else:
		os.system('echo "10;NewKaku;FFFFFE;1;ON" > /dev/ttyUSB0')

	logger.info("Set alarm status from Web interface " + command)


db = c.execute('select * from alarm')
[alarmActivationStatus, alarmIntrusionStatus, alarmAddressOnHome, alarmAddressOnAway, alarmAddressOff] = db.fetchone()

conn.close()

fgcolor = "#505050"
if alarmActivationStatus in ["ON_HOME", "ON_AWAY"]:
   fgcolor = "#800517"

bgcolor = "#10FF10"
if alarmIntrusionStatus == "YES":
   bgcolor = "#F00517"



print "Content-Type: text/html\n\n"
print '<html><head><meta content="text/html; charset=UTF-8" />'
print '<title>My House</title><p>'
print '<body>'

printTopBar()

print '<table border="0" style="font-size:200%;">'
print "<tr>"
print "<td>Alarm status:</td>"
print '<td bgcolor="#909090"> <FONT COLOR="'+ fgcolor + '">' + alarmActivationStatus +'</FONT> </td>'



print "<td>"
print '<form action="alarm.py" method="GET">'
print '<input type="hidden" name="command" value="ON_AWAY">'
print '<input type="submit" value="Arm Away">'
print '</form>'
print "</td>"


print "<td>"
print '<form action="alarm.py" method="GET">'
print '<input type="hidden" name="command" value="ON_HOME">'
print '<input type="submit" value="Arm Home">'
print '</form>'
print "</td>"


print "<td>"
print '<form action="alarm.py" method="GET">'
print '<input type="hidden" name="command" value="OFF">'
print '<input type="submit" value="OFF">'
print '</form>'
print "</td>"

print "</tr>"

print "<tr>"
print "<td>Intrusion:</td>"
print '<td colspan="3" bgcolor="' + bgcolor + '">'+ alarmIntrusionStatus +'</td>'
print "</tr>"


print "</table>" 


print "</body></html>"







