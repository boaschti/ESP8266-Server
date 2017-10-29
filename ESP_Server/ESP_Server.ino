#include "ESPServer.h"


#define SERIAL_BAUD   57600

/*
void UserSubscribe(){
    #ifndef usersubscribe_existing
    #define usersubscribe_existing
    #endif
    return;  
}
void MQTTBrokerChanged(){
    #ifndef MQTTBrokerChanged_existing
    #define MQTTBrokerChanged_existing
    #endif
    return;  
}


#ifndef userpage_existing
#define userpage_existing
#endif 


    static const char PROGMEM USERPAGE_HTML[] = R"rawliteral(
    <!DOCTYPE html>
    <html>

    </html>
    )rawliteral";

    void handleconfigureUserWrite(){
        handleRoot();
    }
    void handleconfigureUser(){
        webServersend_P(200, "text/html", USERPAGE_HTML);
    }

*/


void callback(char* topic, byte* payload, unsigned int length) {
    mqttpublish("Info", "got Message");
}


void setup(){
    Serial.begin(SERIAL_BAUD);
    setup_server();
    
}

void loop(){
  
     loop_server();
  
}