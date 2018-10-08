#! /usr/bin/python
#
from topbar import printTopBar

print "Content-Type: text/html\n\n"
print '<html><head><meta content="text/html; charset=UTF-8" />'
print '<title>My House</title><p>'
print '<body>'

printTopBar()

print '<table border="0" style="font-size:200%;">'
print "<tr>"
print "<td>Alarm status</td>"
print '<td bgcolor="#7F7F7F">ON</td>'



print "<td>"
print '<form name="input" action="alarm.py" method="get">'
print '<input type="submit" value="Arm Away">'
print '</form>'
print "</td>"

print "<td>"
print '<form name="input" action="alarm.py" method="get">'
print '<input type="submit" value="Arm Home">'
print '</form>'
print "</td>"

print "<td>"
print '<form name="input" action="alarm.py" method="get">'
print '<input type="submit" value="OFF">'
print '</form>'
print "</td>"


print "</tr>"

print "<tr>"
print "<td>Intrusion</td>"
print '<td colspan="3" bgcolor="#00FF00">NO</td>'
print "</tr>"


#for line in open('/home/pi/record'):
#  lineSpl = line.split(' ')
#  print "<tr>"
#  print "<td>"+lineSpl[0]+"</td>"
#  print "<td>"+lineSpl[1]+"</td>"
#  print "<td>"+lineSpl[2]+"</td>"
#  print "</tr>"

print "</table>" 


print "</body></html>"







