#include "mbed.h"
#include "Servo.h"
#include "uLCD_4DGL.h"

//include server stuff

Servo myservo(p21);
uLCD_4DGL LCD(p28,p27,p30);

enum Statetype {Heat_on, Cool_on, off};
float servomax = 1;
float servomin = 0;
float servohalf;

int main(){
	Statetype state = off;
	servohalf = (servomax + servomin)/2;
	int current_temp;
	int target_temp;
	char current_temp_string[3];
	char target_temp_string[3];
	
	LCD.background_color(BLACK);
	
	while (1) {
		//need a get state from server
		LCD.locate(2,3);
		sprintf(current_temp_string, "%i", current_temp);
		LCD.puts(current_temp_string);
		LCD.locate(2,5);
		sprintf(target_temp_string, "%i", target_temp);
		LCD.puts(target_temp_string);
		LCD.locate(2,7);
		switch (state) {
			case Heat_on:
				myservo = servomax;
				LCD.puts("Heating");
				break;
			case Cool_on:
				myservo = servomin;
				LCD.puts("Cooling");
				break;
			case off:
				myservo = servomin;
				LCD.puts("Off    ");
				break;
		}
	}
    return 0;
}