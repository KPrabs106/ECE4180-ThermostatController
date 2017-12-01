#include "mbed.h"
#include "Servo.h"
#include "uLCD_4DGL.h"

//include server stuff

Servo myservo(p21);
enum Statetype {Heat_on, Cool_on, off};
float servomax = 1;
float servomin = 0;
float servohalf;

int main(){
	Statetype state = off;
	servohalf = (servomax + servomin)/2;
	while (1) {
		//need a get state from server
		switch (state) {
			case Heat_on:
				myservo = servomax;
				break;
			case Cool_on:
				myservo = servomin;
				break;
			case off:
				myservo = servomin;
				break;
		}
	}
    return 0;
}