#include "mbed.h"
#include "uLCD_4DGL.h"
#include "easy-connect.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "credentials.h"
#include <string>
#include <iostream>
using namespace std;

int Current_temp = 0;
int Target_temp = 75;
int Schedule_temp = 0;

uLCD_4DGL uLCD(p28, p27, p29); // create a global uLCD object

void messageArrived(MQTT::MessageData& md) // temperature: Current Temperature from Slave Device
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    Current_temp = atoi((char*)message.payload);
}

void messageArrived2(MQTT::MessageData& md) // ideal: Ideal Temperature for Override Mode
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    Target_temp = atoi((char*)message.payload);
}

void messageArrived3(MQTT::MessageData& md) // control: On or Off
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    Target_temp = atoi((char*)message.payload);
}

void messageArrived4(MQTT::MessageData& md) // control2: Override or Schedule
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    Target_temp = atoi((char*)message.payload);
}

void messageArrived5(MQTT::MessageData& md) // schedule
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    std::string schedule = ((char*)message.payload);

    std::string d1 = ";";
    std::string d2 = ", ";
    std::string d3 = ":";

    size_t pos = 0;
    std::string token1;
    std::string token2;
    std::string token3;
    int hour;
    int min;
    int temperature;
    while ((pos = schedule.find(d1)) != std::string::npos) {
        printf("enter, %c\r\n", std::string::npos);
        token1 = schedule.substr(0, 9);//pos);
        printf("enter2, %c\r\n", std::string::npos);
        token2 = token1.substr(0, 5);//token1.find(d2));
        printf("enter3, %s\r\n", token2);
        token3 = token2.substr(0, 2);//token2.find(d3));
        printf("after token\r\n");
        hour = atoi(token3.c_str());
        token2.erase(0, token3.length() + d3.length());
        min = atoi(token2.c_str());
        token1.erase(0, token3.length() + d3.length() + token2.length() + d2.length());
        temperature = atoi(token1.c_str());
        printf("before erase\r\n");
        schedule.erase(0, pos + d1.length());
        printf("%i:%i, %i\r\n", hour, min, temperature);
    }
}

int temp_hour = 9;
int temp_min = 15;

enum Modetype { MODE_OFF = 0, HEAT, COOL };
Modetype mode = MODE_OFF;

int main()
{
    set_time(1356735557);
    uLCD.baudrate(BAUD_3000000);

    extern const char* hostname;
    extern const int port;
    extern char* username;
    extern char* password;

    NetworkInterface* network = easy_connect(true);
    MQTTNetwork mqttNetwork(network);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\r\n", rc);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;

    char clientID[100];
    strcpy(clientID, "mbed-");
    strcat(clientID, network->get_mac_address());

    data.clientID.cstring = clientID;
    data.username.cstring = username;
    data.password.cstring = password;
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\r\n", rc);

    //if ((rc = client.subscribe("test/a", MQTT::QOS2, messageArrived)) != 0)
    //    printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("temperature", MQTT::QOS2, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("ideal", MQTT::QOS2, messageArrived2)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("control", MQTT::QOS2, messageArrived3)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("control2", MQTT::QOS2, messageArrived4)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("schedule", MQTT::QOS2, messageArrived5)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    MQTT::Message message;

    // QoS 0
    char buf[100];
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;

    enum Statetype { STATE_OFF = 0, HEAT_OFF, HEAT_ON, COOL_OFF, COOL_ON };
    Statetype state = STATE_OFF;
    uLCD.background_color(0x008000);
    uLCD.textbackground_color(0x008000);
    uLCD.color(WHITE);
    uLCD.cls();
    uLCD.locate(7,7);
    uLCD.printf("Off");
    while(1) {
        time_t seconds = time(NULL);
        uLCD.locate(0,14);
        tm * now = localtime( &seconds);
        client.yield(100); // Update Current_temp and Target_temp
        int current_hour = now -> tm_hour;
        switch (state) {
            case STATE_OFF:
                if (mode == HEAT) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = HEAT_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Heat");
                    break;
                }
                if (mode == COOL) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = COOL_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Cool");
                    break;
                }
                break;
            case HEAT_OFF:
                if (mode == MODE_OFF) {
                    uLCD.background_color(0x008000);
                    uLCD.textbackground_color(0x008000);
                    uLCD.cls();
                    state = STATE_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Off");
                    break;
                }
                if (mode == COOL) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = COOL_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Cool");
                    break;
                }
                if (Current_temp < (Target_temp - 1)) {
                    uLCD.background_color(RED);
                    uLCD.textbackground_color(RED);
                    uLCD.cls();
                    state = HEAT_ON;
                    uLCD.locate(5,7);
                    uLCD.printf("Heating");
                    break;
                }
                break;
            case HEAT_ON:
                if (mode == MODE_OFF) {
                    uLCD.background_color(0x008000);
                    uLCD.textbackground_color(0x008000);
                    uLCD.cls();
                    state = STATE_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Off");
                    break;
                }
                if (mode == COOL) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = COOL_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Cool");
                    break;
                }
                if (Current_temp > (Target_temp + 1)) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = HEAT_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Heat");
                    break;
                }
                break;
            case COOL_OFF:
                if (mode == MODE_OFF) {
                    uLCD.background_color(0x008000);
                    uLCD.textbackground_color(0x008000);
                    uLCD.cls();
                    state = STATE_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Off");
                    break;
                }
                if (mode == HEAT) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = HEAT_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Heat");
                    break;
                }
                if (Current_temp > (Target_temp + 1)) {
                    uLCD.background_color(BLUE);
                    uLCD.textbackground_color(BLUE);
                    uLCD.cls();
                    state = COOL_ON;
                    uLCD.locate(5,7);
                    uLCD.printf("Cooling");
                    break;
                }
                break;
            case COOL_ON:
                if (mode == MODE_OFF) {
                    uLCD.background_color(0x008000);
                    uLCD.textbackground_color(0x008000);
                    uLCD.cls();
                    state = STATE_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Off");
                    break;
                }
                if (mode == HEAT) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = HEAT_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Heat");
                    break;
                }
                if (Current_temp < (Target_temp - 1)) {
                    uLCD.background_color(BLACK);
                    uLCD.textbackground_color(BLACK);
                    uLCD.cls();
                    state = COOL_OFF;
                    uLCD.locate(7,7);
                    uLCD.printf("Cool");
                    break;
                }
                break;
        }
        uLCD.locate(6,2);
        uLCD.printf("Target");
        uLCD.locate(6,9);
        uLCD.printf("Current");
        uLCD.locate(8,11);
        uLCD.text_height(2);
        uLCD.printf("%I" , Current_temp);
        uLCD.locate(8,2);
        uLCD.printf("%I" , Target_temp);
        uLCD.text_height(1);
        uLCD.locate(0,14);
        uLCD.printf("     %I:%02I:%02I" , current_hour, now->tm_min, now->tm_sec);
        wait(0.33);
    }

    return 0;
}

