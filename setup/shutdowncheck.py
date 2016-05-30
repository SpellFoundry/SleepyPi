#!/usr/bin/python

import RPi.GPIO as GPIO
import os, time

GPIO.setmode(GPIO.BCM)
GPIO.setup(24, GPIO.IN)
GPIO.setup(25, GPIO.OUT)
GPIO.output(25, GPIO.HIGH)
print ("[Info] Telling Sleepy Pi we are running pin 25")

while True:
	if (GPIO.input(24)):
		print ("Sleepy Pi requesting shutdown on pin 24")
		os.system("sudo shutdown -h now")
		break
	time.sleep(0.5)
