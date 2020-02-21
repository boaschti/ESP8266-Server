

#define userpage_existing
//#define AllowAcessPoint

#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESPServer.h"
#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESPServer.cpp"
#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESP8266HTTPUpdateServer.h"
#include "C:\Users\Basti\Documents\ESP8266-Server\ESP_Server\ESP8266HTTPUpdateServer.cpp"

  

//#define AllowAcessPoint         //opens a acessPoint if saved Wlan is not reachable


/*
#define usersubscribe_existing
void UserSubscribe(){
    mqttpublish("Info", "usersubscribe");
}
*/

/*
//#define MQTTBrokerChanged_existing
void MQTTBrokerChanged(){
    return;  
}
*/


    static const char PROGMEM USERPAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name = 'viewport' content = 'width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0'>
  <title>RFM69 Node Configuration</title>
  <style>
    'body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }'
  </style>
</head>
<body>

    <p><a href="/"><button type="button">Home</button></a>
</body>
</html>
)rawliteral";

void handleconfigureUserWrite(){
    handleRoot();
}
void handleconfigureUser(){
    webServersend_P(USERPAGE_HTML);
}


void callback(char* topic, byte* payload, unsigned int length) {
    mqttpublish("Info", "got Message");
}


void setup(){
    setup_server();
    
}

void loop(){
     loop_server();
}