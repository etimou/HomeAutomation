#! /usr/bin/python

import socket
import sys
command = sys.argv[1]


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 50000))
s.sendall(command + '\n')
data = s.recv(1024)
s.close()

if data == command:
  exit(0)
else:
  exit(1)
