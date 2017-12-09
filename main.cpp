#include "mbed.h"
#include "uLCD_4DGL.h"
#include "easy-connect.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "credentials.h"

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

int main(){
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

    if ((rc = client.subscribe("test/a", MQTT::QOS2, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
 
    MQTT::Message message;
 
    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f\r\n", 1);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish("test/a", message);
    while (arrivedcount < 1)
        client.yield(100);
    
    while(true){

    }

	return 0;
}

/**
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
**/
