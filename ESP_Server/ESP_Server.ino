#include "ESPServer.h"


#define SERIAL_BAUD   57600

/*
void UserSubscribe(){
    #ifndef usersubscribe_existing
    #define usersubscribe_existing
    return;  
}
void MQTTBrokerChanged(){
    #ifndef MQTTBrokerChanged_existing
    #define MQTTBrokerChanged_existing
    return;  
}
*/


void callback(char* topic, byte* payload, unsigned int length) {
    return;
}


void setup(){
    Serial.begin(SERIAL_BAUD);
    setup_server();
    
}

void loop(){
  
     loop_server();
  
}