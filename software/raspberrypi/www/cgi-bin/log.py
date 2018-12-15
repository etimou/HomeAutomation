#! /usr/bin/python
#
from topbar import printTopBar

print "Content-Type: text/html\n\n"
print '<html><head><meta content="text/html; charset=UTF-8" />'

print '<title>My House</title><p>'
print '<body>'

printTopBar()

file=open("/home/pi/HomeAutomation/software/raspberrypi/logging.log", 'r') 

print  "<textarea>"
for line in file:
	print line[:-1]

file.close()
print "</textarea>" 

print "</body></html>"







