#include "mbed.h"

int main() {
	set_time(1256729737); //Number of seconds since January 1, 1970 (the UNIX timestamp)

	while (true) {
		time_t seconds = time(NULL);
		char buffer[32];
		strftime(buffer, 32, "%H:%M", localtime(seconds)); // %H is hours in 24hr format, %M is minutes 0-59 in strftime
		printf("%s", buffer);
		wait(1);
	}
}