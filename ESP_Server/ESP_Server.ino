#include "ESPServer.h"



/*
void UserSubscribe(){
    mqttpublish("Info", "usersubscribe");
}

void MQTTBrokerChanged(){
    return;  
}

    static const char PROGMEM USERPAGE_HTML[] = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
      <title>DeviceName MQTT Configuration</title>
      <style>
        "body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
      </style>
    </head>
    <body>
      <h3>Hi Userpage is here!</h3>
      <a href="/"><button type="button">Home</button></a>
    </body>
    </html>
    )rawliteral";

    void handleconfigureUserWrite(){
        handleRoot();
    }
    void handleconfigureUser(){
        webServersend_P(USERPAGE_HTML);
    }


void handleconfigureUserWrite(){
    handleRoot();
}
void handleconfigureUser(){
    webServersend_P(USERPAGE_HTML);
}
*/

void callback(char* topic, byte* payload, unsigned int length) {
    mqttpublish("Info", "got Message");
}


void setup(){
    setup_server();
    
}

void loop(){
  
     loop_server();
  
}