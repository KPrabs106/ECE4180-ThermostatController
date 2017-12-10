#include "mbed.h"
#include "EthernetNetIf.h"
#include "NTPClient.h"

EthernetNetIf eth;
NTPClient ntp;
 
int main() {
    time_t ctTime;
    EthernetErr ethErr = eth.setup();
    if (ethErr) {
        //error getting an IP address
        return -1;
    }
    //time server
    Host server(IpAddr(), 123, "0.uk.pool.ntp.org");
    //Read time from server
    ntp.setTime(server);
    while (1) {
        ctTime = time(NULL);
        //printf("UTC:  %s", ctime(&ctTime)); //this makes a formatted string, which you can use delimiters to grab the hour and minute from

        //alternatively
        char buf[64];
        strftime(buf, sizeof(buf), "%H:%M\n", tmp); // puts the hour and minute into buf, just like other time.cpp
        wait(1);
    }
}