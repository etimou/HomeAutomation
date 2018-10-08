#! /usr/bin/python
#

def printTopBar():

	print '<table border="0">'

	print "<td>"
	print '<form name="input" action="sensors.py" method="get">'
	print '<input style="font-size:200%;" type="submit" value="Voir les infos capteurs">'
	print '</form>'
	print "</td>"

	print "<td>"
	print '<form name="input" action="actuators.py" method="get">'
	print '<input style="font-size:200%;" type="submit" value="Actionner">'
	print '</form>'
	print "</td>"

	print "<td>"
	print '<form name="input" action="alarm.py" method="get">'
	print '<input style="font-size:200%;" type="submit" value="Alarme">'
	print '</form>'
        print "</td>"

	#print "</tr>"
	print "</table>" 

	print "<hr>"









