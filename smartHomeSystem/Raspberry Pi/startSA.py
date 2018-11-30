from Adafruit_MotorHAT import Adafruit_MotorHAT, Adafruit_DCMotor, Adafruit_StepperMotor

import time
import RPi.GPIO as GPIO
import atexit


##### onLight
GPIO.setmode(GPIO.BCM)

print "Setup what?"

GPIO.setup(21, GPIO.OUT)
GPIO.output(21, False)

GPIO.output(21, True)
time.sleep(2)
#GPIO,output(21, False)
GPIO.cleanup()


##### upBlind 
print "start up blind"
mh = Adafruit_MotorHAT(addr = 0x60)

def turnOffMotors():
        mh.getMotor(1).run(Adafruit_MotorHAT.RELEASE)
        mh.getMotor(2).run(Adafruit_MotorHAT.RELEASE)
        mh.getMotor(3).run(Adafruit_MotorHAT.RELEASE)
        mh.getMotor(4).run(Adafruit_MotorHAT.RELEASE)

atexit.register(turnOffMotors)

myStepper = mh.getStepper(200, 1)
myStepper.setSpeed(20000)

print("Double coil steps")
myStepper.step(700, Adafruit_MotorHAT.FORWARD,  Adafruit_MotorHAT.DOUBLE)
