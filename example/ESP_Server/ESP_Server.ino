

#define userpage_existing
//#define userpage2_existing

//#define AllowAcessPoint         // opens a acessPoint if saved Wlan is not reachable
//#define WiFiNotRequired         // if there is no connection -> return from setup_server() (and run main loop) or restart esp until connection
//#define showKeysInWeb true      // shows keys in WEB page (for debug! not recommendet!!)

//#define usersubscribe_existing  //if a UserSubscribe() exists. Else the Server will connect automatically to set Topic
//#define MQTTBrokerChanged_existing  // if a MQTTBrokerChanged() exists you can do sth if Broker is changed via web

#define DeviceName "ESPServerTest"
#define UserPageName "Testpage"

#include "C:\Users\sebas\Documents\ESP8266-Server\ESPServer.h"

/*
void UserSubscribe(){
    mqttpublish("Info", "usersubscribe");
}
*/

/*
void MQTTBrokerChanged(){
    return;  
}
*/

//######################### EEPROM #########################
//EEProm Access 1. Example
/*
typedef struct {
  char        encryptkey[16+1];
  uint8_t     networkid;
  uint8_t     nodeid;
  uint8_t     powerlevel; // bits 0..4 power level, bit 7 RFM69HCW 1=true
  uint8_t     rfmfrequency;
} userEEProm_mt;
userEEProm_mt userEEProm;

userEEProm_mt * pUserData;


void UserEEPromSetup(void* eepromPtr) {
      userEEProm_mt* pUserData = (userEEProm_mt*)eepromPtr;
      Serial.println("ENCRYPTKEY");
      strcpy_P(pUserData->encryptkey, ENCRYPTKEY);
      Serial.println("NETWORKID");
      pUserData->networkid = NETWORKID;
      Serial.println("NODEID");
      pUserData->nodeid = NODEID;
      Serial.println("powerlevel");
      pUserData->powerlevel = ((IS_RFM69HCW)?0x80:0x00) | POWER_LEVEL;
      Serial.println("rfmfrequency");
      pUserData->rfmfrequency = FREQUENCY;
};

uint32_t UserEEPromChecksum(uint32_t checksum, void* eepromPtr) {
        uint8_t * pUserData = (uint8_t *)eepromPtr;

        for (uint32_t i = 0; i < sizeof(userEEProm_mt); i++)
        {
            checksum += *pUserData++;
        }
    
        return checksum;
};


read value:
x = pUserData->networkid

write and save value:
pUserData->networkid = x
SaveEEpromData();


*/

//EEprom access 2. Example
/*

pGC->userMem32[0..19] = data
pGC->userMem8[0..49] = data
pGC->checksum = gc_checksum()
EEPROM.commit()

read value:
pGC->userMem32[NumberofLedsEEprom]

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