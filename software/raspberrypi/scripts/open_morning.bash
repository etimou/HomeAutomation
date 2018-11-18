#!/bin/bash
echo "10;X10;000041;a;UP" > /dev/ttyUSB0
sleep 10
echo "10;X10;000041;a;STOP" > /dev/ttyUSB0
