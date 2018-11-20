#! /usr/bin/python
#
from topbar import printTopBar
import sqlite3
import os


command= os.environ['QUERY_STRING']
if len(command)>=8:
	#command = 'echo "' + command[8:].replace('%3B',';') + '" > /home/eti/HomeAutomation/software/raspberrypi/out.log'
        command = 'echo "' + command[8:].replace('%3B',';') + '" > /dev/ttyUSB0'
	os.system(command)


print "Content-Type: text/html\n\n"
print '<html><head><meta content="text/html; charset=UTF-8" />'
print '<title>My House</title><p>'
print '<body>'

printTopBar()

print command

print '<table border="1" style="font-size:200%;">'
print "<tr>"
print "<td>ID</td>"
print "<td>Name</td>"
print "<td>Action 1</td>"
print "<td>Action 2</td>"
print "<td>Action 3</td>"
print "</tr>"

conn = sqlite3.connect('/home/pi/HomeAutomation/software/raspberrypi/MyHouse.db')
c = conn.cursor()

for row in c.execute('SELECT * FROM actuators ORDER BY ID'):
	print "<tr>"
	print "<td>"+str(row[0])+"</td>"
	print "<td>"+row[1]+"</td>"

	print "<td>"
	print '<form action="actuators.py" method="GET">'
  	print '<input type="hidden" name="command" value="'+row[3]+'">'
  	print '<input type="submit" value="'+row[2]+'">'
	print '</form>'
        print "</td>"

	print "<td>"
	print '<form action="actuators.py" method="GET">'
  	print '<input type="hidden" name="command" value="'+row[5]+'">'
  	print '<input type="submit" value="'+row[4]+'">'
	print '</form>'
        print "</td>"

	print "<td>"
	if row[6]!=None:
		print '<form action="actuators.py" method="GET">'
  		print '<input type="hidden" name="command" value="'+row[7]+'">'
  		print '<input type="submit" value="'+row[6]+'">'
		print '</form>'
        print "</td>"


	print "</tr>"

conn.close()

print "</table>" 



print "</body></html>"







