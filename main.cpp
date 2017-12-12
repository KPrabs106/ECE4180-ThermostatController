#include "mbed.h"
#include "uLCD_4DGL.h"
#include "easy-connect.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "credentials.h"
#include "Motor.h"

using namespace std;

int currentTemp = 0;
int targetTemp = 75;
int threshold = 1;

Motor m(p23, p5, p6); // pwm, fwd, rev - top motor
Motor m2(p24, p9, p8); // pwm, fwd, rev - bottom motor
Motor m3(p25, p12, p13); // pwm, fwd, rev - knob motor
DigitalIn pb1(p22); // Increase temp
DigitalIn pb2(p21); // Decrease temp

enum Modetype { MODE_OFF = 0, MODE_ON };
Modetype mode = MODE_OFF;
enum Statetype { HEAT_OFF = 0, HEAT_ON, COOL_OFF, COOL_ON };
Statetype state = HEAT_OFF;

uLCD_4DGL uLCD(p28, p27, p29); // create a global uLCD object

void messageArrived(MQTT::MessageData& md) // temperature: Current Temperature from Slave Device
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

    // can't just use (char*)message.payload, some weird stuff gets printed at the end
    char buf[5];
    sprintf(buf, "%.*s", message.payloadlen, (char*)message.payload);

    currentTemp = atoi(buf);
    uLCD.locate(8,11);
    uLCD.text_height(2);
    printf("%d\r\n", currentTemp);
    uLCD.printf("%d     " , currentTemp);
    uLCD.text_height(1);
}

void messageArrived2(MQTT::MessageData& md) // ideal: Ideal Temperature for Override Mode
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

    // can't just use (char*)message.payload, some weird stuff gets printed at the end
    char buf[5];
    sprintf(buf, "%.*s", message.payloadlen, (char*)message.payload);

    targetTemp = atoi(buf);
    uLCD.locate(8,4);
    uLCD.text_height(2);
    uLCD.printf("%d      " , targetTemp);
    uLCD.text_height(1);
}

void messageArrived3(MQTT::MessageData& md) // control: On or Off
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

    // can't just use (char*)message.payload, some weird stuff gets printed at the end
    char buf[5];
    sprintf(buf, "%.*s", message.payloadlen, (char*)message.payload);

    mode = (Modetype)atoi(buf);
}

void controlHeat()  // move knob to heat
{
    m3.speed(-1.0);
    wait(1);
    m3.speed(0);
}

void controlCool()  // move knob to cool
{
    m3.speed(1.0);
    wait(1);
    m3.speed(0);
}

void controlOn()  // turn switch on
{
    m.speed(-1.0);
    wait(.5);
    m.speed(1.0);
    wait(.5);
    m.speed(-1.0);
    wait(.5);
    m.speed(1.0);
    wait(.5);
    m.speed(-1.0);
    wait(1);
    m.speed(0.5);
    wait(.25);
    m.speed(0);
}

void controlOff()  // turn switch off
{
    m2.speed(1.0);
    wait(.5);
    m2.speed(-1.0);
    wait(.5);
    m2.speed(1.0);
    wait(.5);
    m2.speed(-1.0);
    wait(.5);
    m2.speed(1.0);
    wait(1);
    m2.speed(-0.5);
    wait(.25);
    m2.speed(0);
}

//void pb1_hit_callback (void)
//{
//    char buf[100];
//    sprintf(buf, "%d", currentTemp + 1);
//    messagePb.payload = (void*)buf;
//    messagePb.payloadlen = strlen(buf)+1;
//    pbFlag = true;
//    printf("Increase temp flag set.\r\n");
//    //client.publish("temperature", messagePb);
//}

//void pb2_hit_callback (void)
//{
//    char buf[100];
//    sprintf(buf, "%d", currentTemp - 1);
//    messagePb.payload = (void*)buf;
//    messagePb.payloadlen = strlen(buf)+1;
//    pbFlag = true;
//    printf("Decrease temp flag set.\r\n");
//    //client.publish("temperature", messagePb);
//}

int main()
{
    pb1.mode(PullUp);
    pb2.mode(PullUp);
    wait(.001);
//    pb1.attach_deasserted(&pb1_hit_callback);
//    pb2.attach_deasserted(&pb2_hit_callback);
//    pb1.setSampleFrequency();
//    pb2.setSampleFrequency();

    uLCD.baudrate(BAUD_3000000);
    uLCD.cls();

    uLCD.locate(6,2);
    uLCD.printf("Target");
    uLCD.locate(6,9);
    uLCD.printf("Current");

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

    if ((rc = client.subscribe("temperature", MQTT::QOS2, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("ideal", MQTT::QOS2, messageArrived2)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe("control", MQTT::QOS2, messageArrived3)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);

    MQTT::Message messagePb;
    messagePb.qos = MQTT::QOS0;
    messagePb.retained = true;
    messagePb.dup = false;

    while(true) {
        //printf("start yield\r\n");
        client.yield(100);
        //printf("end yield\r\n");
        // if(client.yield(100) == -1){ // disconnected, try reconnecting
        //  if ((rc = client.connect(data)) != 0)
        //      printf("rc from MQTT connect is %d\r\n", rc);
        //  if ((rc = client.subscribe("temperature", MQTT::QOS2, messageArrived)) != 0)
        //      printf("rc from MQTT subscribe is %d\r\n", rc);
        //  if ((rc = client.subscribe("ideal", MQTT::QOS2, messageArrived2)) != 0)
        //      printf("rc from MQTT subscribe is %d\r\n", rc);
        // }

        if (pb1 == 0) {
            char buf[100];
            sprintf(buf, "%d", currentTemp + 1);
            messagePb.payload = (void*)buf;
            messagePb.payloadlen = strlen(buf)+1;
            printf("Increase temp flag set.\r\n");
            client.publish("temperature", messagePb);
            wait(.5);
        }
        if (pb2 == 0) {
            char buf[100];
            sprintf(buf, "%d", currentTemp - 1);
            messagePb.payload = (void*)buf;
            messagePb.payloadlen = strlen(buf)+1;
            printf("Decrease temp flag set.\r\n");
            client.publish("temperature", messagePb);
            wait(.5);
        }

        printf("Mode: %d\r\n", mode);
        printf("State: %d\r\n", state);

        if(mode == MODE_OFF) { // don't do anything
            continue;
        }


        if(targetTemp > currentTemp+threshold) { // need to heat
            printf("Need to heat!\r\n");
            switch(state) {
                case HEAT_OFF:
                    controlOn();
                    state = HEAT_ON;
                    break;
                case COOL_OFF:
                    controlHeat();
                    controlOn();
                    state = HEAT_ON;
                    break;
                case COOL_ON:
                    controlHeat();
                    state = HEAT_ON;
                    break;
            }
        } else if(targetTemp < currentTemp-threshold) { // need to cool
            printf("Need to cool!\r\n");
            switch(state) {
                case COOL_OFF:
                    controlOn();
                    state = COOL_ON;
                    break;
                case HEAT_OFF:
                    controlCool();
                    controlOn();
                    state = COOL_ON;
                    break;
                case HEAT_ON:
                    controlCool();
                    state = COOL_ON;
                    break;
            }
        } else { // in range, so turn off if anything is on
            printf("Do nothing!\r\n");
            switch(state) {
                case HEAT_ON:
                    controlOff();
                    state = HEAT_OFF;
                    break;
                case COOL_ON:
                    controlOff();
                    state = COOL_OFF;
                    break;
            }
        }
    }
    return 0;
}
