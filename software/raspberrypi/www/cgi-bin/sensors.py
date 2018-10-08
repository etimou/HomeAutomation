#! /usr/bin/python
#
from topbar import printTopBar
import sqlite3

print "Content-Type: text/html\n\n"
print '<html><head><meta content="text/html; charset=UTF-8" />'

print '<title>My House</title><p>'
print '<body>'

printTopBar()

print '<table border="1" style="font-size:200%;">'
print "<tr>"
print "<td>ID</td>"
print "<td>Name</td>"
print "<td>status</td>"
print "<td>last seen</td>"
print "<td>battery level</td>"
print "<td>address</td>"
print "<td>type</td>"
print "<td>InAlarmHome</td>"
print "<td>InAlarmAway</td>"
print "</tr>"

conn = sqlite3.connect('/home/eti/MyHouse.db')
c = conn.cursor()

for row in c.execute('SELECT * FROM sensors ORDER BY ID'):
	print "<tr>"
	print "<td>"+str(row[0])+"</td>"
	print "<td>"+row[1]+"</td>"
	print "<td>"+str(row[2])+"</td>"
	print "<td>"+row[3][:-4]+"</td>"
	print "<td>"+str(row[4])+"%</td>"
	print "<td>"+str(row[5])+"</td>"
	print "<td>"+str(row[6])+"</td>"
	print "<td>"+str(row[7])+"</td>"
	print "<td>"+str(row[8])+"</td>"
	print "</tr>"

conn.close()

print "</table>" 


print "</body></html>"







