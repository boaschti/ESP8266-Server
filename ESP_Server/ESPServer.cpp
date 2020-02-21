
//This Code generates a Webserver on ES8266 which provides
//- every value is changeable via browser
//- Ota setup via browser file upload with Password
//- MQTT connection
//- setup pages incl placeholder for a userconfigpage (handleconfigureUserWrite() and handleconfigureUser())
//- mdns setup
//- you can use pGC->tx_topic, pGC->rx_topic in you main code
//- you can create a UserSubscribe() funktion to make your own subsribe code
//- you can create a MQTTBrokerChanged() funktion



//- you only have to include this File and call setup_server()
//- oyu have to make a funktion to get messages: void callback(char* topic, byte* payload, unsigned int length) {}


//  todo
//- you can add your DeviceName by search and replace
//- you can add your UserPageName by search and replace
//- WifiManager.h move private: int           connectWifi(String ssid, String pass);  to public: int           connectWifi(String ssid, String pass);

/*
#define DEVICE_NAME   "FOOBAR"


const char xyz[] = "ABC" "XYZ";
const char xyz[] = "ABC" DEVICE_NAME #  "XYZ";
*/


#define SERIAL_BAUD   115200

#include "ESPServer.h"

#include <ESP8266WiFi.h>
#include <pgmspace.h>
#include <PubSubClient.h>



// Default values
const char PROGMEM MDNS_NAME[] = "DeviceName";
const char PROGMEM MQTTCLIENTNAME[] = "DeviceName";
const char PROGMEM MQTT_BROKER[] = "raspberrypi";
const char PROGMEM RX_TOPIC[] = "DeviceName/set/#";
const char PROGMEM TX_TOPIC[] = "DeviceName/state";
const char PROGMEM AP_NAME[] = "DeviceName-AP";

//Passwort und Benutzer zum Softwareupdate ueber Browser
const char PROGMEM UPDATEUSER[]		  = "B";
const char PROGMEM UPDATEPASSWORD[] =	"B";
const char PROGMEM MQTTPASSWORD[]   = "B";
const char PROGMEM MQTTUSER[]   = "B";

#define showKeysInWeb true
//#define showKeysInWeb false

#define HiddenString "xxx"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// vvvvvvvvv Global Configuration vvvvvvvvvvv
#include <EEPROM.h>

void callback(char* topic, byte* payload, unsigned int length);

#ifdef MQTTBrokerChanged_existing
    void MQTTBrokerChanged();
#endif 

#ifdef usersubscribe_existing
    void UserSubscribe();
#endif

struct _GLOBAL_CONFIG {
    uint32_t    checksum;
    char        apname[32];
    char        mqttbroker[32];
    char        mqttclientname[32];
    char        rx_topic[32];
    char        tx_topic[32];
    char        mdnsname[32];
    uint32_t    ipaddress;  // if 0, use DHCP
    uint32_t    ipnetmask;
    uint32_t    ipgateway;
    uint32_t    ipdns1;
    uint32_t    ipdns2;
    char        updateUser[20];
    char        updatePassword[20];
    char        mqttPassword[20];
    char        mqttUser[20];  
    //uint8_t     userMem[1024];
};

struct _GLOBAL_CONFIG *pGC;
// ^^^^^^^^^ Global Configuration ^^^^^^^^^^^

// vvvvvvvvv ESP8266 WiFi vvvvvvvvvvv
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


WiFiManager wifiManager;

boolean DeviceEnteredConfigAp = false;

void configModeCallback (WiFiManager *myWiFiManager) {
    Serial.println("SW: Entered AP to config");
    DeviceEnteredConfigAp = true;
}

void wifi_setup(void) {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  //reset settings - for testing. Wipes out SSID/password.
  //wifiManager.resetSettings();
 
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  /*
  #ifdef AllowAcessPoint
      wifiManager.setConfigPortalTimeout(180);
  #else
      // try to connect 3 times then go in a blocking loop for config AP
      wifiManager.setConfigPortalTimeout(1);
      if(!wifiManager.autoConnect(pGC->apname)){
        
      }
      wifiManager.setConfigPortalTimeout(180);
  #endif
  
  wifiManager.setConfigPortalTimeout(1);
  */
  
  
  
  wifiManager.setConfigPortalTimeout(1);
  
  uint8_t i = 0;
  bool connected = false;
  while((i <= 4)) {
    i++;
    Serial.println("SW: Try to connect");
    
    //if (wifiManager.autoConnect(pGC->apname)){
    if (wifiManager.connectWifi("","") == WL_CONNECTED){
        connected = true;
        Serial.println("SW: connected to saved WLAN dont try again");
        break;        
    }else{
        DeviceEnteredConfigAp = false;
    }
  }
  
  if (!connected){
      Serial.println("SW: start Config Portal because we have no connection to saved wlan");
      wifiManager.setConfigPortalTimeout(360);
      wifiManager.autoConnect(pGC->apname);
  }
  
  // if AP is active we have to reset the Gateway because the wifimanager sends a open AP
  if (DeviceEnteredConfigAp){
      #ifndef AllowAcessPoint
          //wifiManager.mode(WIFI_STA); //see https://github.com/kentaylor/WiFiManager/blob/master/examples/ConfigOnSwitch/ConfigOnSwitch.ino#L46
          Serial.println("SW: reset cause: AP is active");
          Serial.println("SW: Wait 10 seconds");
          Serial.print("current IP: ");
          //Serial.println(wifiManager.localIP());
          //Serial.println(WiFi.localIP();
          delay(10000);
          // ESP.reset dont works first time after serial flashing
          ESP.reset();
          while(1);
      #else
          Serial.println("SW: not connected to saved WLAN. Run Programm.");
      #endif
  }else{
      Serial.println("SW: connected to saved WLAN");
  }
}

// ^^^^^^^^^ ESP8266 WiFi ^^^^^^^^^^^

// vvvvvvvvv Global Configuration vvvvvvvvvvv
uint32_t gc_checksum() {
    uint8_t *p = (uint8_t *)pGC;
    uint32_t checksum = 0;
    p += sizeof(pGC->checksum);
    for (size_t i = 0; i < (sizeof(*pGC) - 4); i++) {
        checksum += *p++;
    }
    return checksum;
}

void eeprom_setup() {
    EEPROM.begin(4096);
    pGC = (struct _GLOBAL_CONFIG *)EEPROM.getDataPtr();
    // if checksum bad init GC else use GC values
    if (gc_checksum() != pGC->checksum) {
    //if (1){  
        Serial.println("Factory reset");
        memset(pGC, 0, sizeof(*pGC));
        Serial.println("AP_NAME");
        strcpy_P(pGC->apname, AP_NAME);
        Serial.println("MQTT_BROKER");
        strcpy_P(pGC->mqttbroker, MQTT_BROKER);
        Serial.println("copy topics");
        strcpy_P(pGC->rx_topic, RX_TOPIC);
        strcpy_P(pGC->tx_topic, TX_TOPIC);
        Serial.println("MDNS_NAME");
        strcpy_P(pGC->mdnsname, MDNS_NAME);
        Serial.println("mqttclientname");
        //bei der folgenden Zeile haengt sich der Chip auf: Exception (28)
        //strcpy(pGC->mqttclientname, WiFi.hostname().c_str());
        strncpy_P(pGC->mqttclientname, MQTTCLIENTNAME,32);
        Serial.println("UPDATEPW");
        strncpy_P(pGC->updateUser, UPDATEUSER, 20);
        strncpy_P(pGC->updatePassword, UPDATEPASSWORD, 20);    
        strncpy_P(pGC->mqttUser, MQTTUSER, 20);
        strncpy_P(pGC->mqttPassword, MQTTPASSWORD, 20);       
        Serial.println("checksum");
        pGC->checksum = gc_checksum();
        Serial.println("EEPROM.commit");
        EEPROM.commit();
    }
}

uint8_t eeprom_read_byte(uint16_t address){
    return 0;
    //return pGC->userMem[address];
}

uint8_t eeprom_write_byte(uint16_t address, uint8_t value){
    //pGC->userMem[address] = value;
    pGC->checksum = gc_checksum();
    //EEPROM.commit();
    return 0;
    //return pGC->userMem[address];
}

// ^^^^^^^^^ Global Configuration ^^^^^^^^^^^

// vvvvvvvvv ESP8266 web sockets vvvvvvvvvvv
#include <ESP8266mDNS.h>


MDNSResponder mdns;

void mdns_setup(void) {
    if (pGC->mdnsname[0] == '\0') return;

    if (mdns.begin(pGC->mdnsname, WiFi.localIP())) {
        Serial.println("MDNS responder started");
        mdns.addService("http", "tcp", 80);
        mdns.addService("ws", "tcp", 81);
    }else{
        Serial.println("MDNS.begin failed");
    }
    Serial.printf("Connect to http://%s or http://", pGC->mdnsname);
    Serial.println(WiFi.localIP());
}


static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>DeviceName</title>
<style>
"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
</style>
<h2>DeviceName</h2>
<div id="configureDeviceName">
  <p><a href="/configDevice"><button type="button">Configure DeviceName</button></a>
</div>
<div id="configureUserPage">
  <p><a href="/configUserPage"><button type="button">Configure UserPageName</button></a>
</div>
<div id="FirmwareUpdate">
  <p><a href="/updater"><button type="button">Update DeviceName Firmware</button></a>
</div>
</body>
</html>
)rawliteral";

static const char PROGMEM CONFIGURE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
  <title>DeviceName Configuration</title>
  <style>
    "body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
  </style>
</head>
<body>
  <h2>DeviceName Configuration</h2>
  <a href="/configDiv"><button type="button">Diverse</button></a>
  <p>
  <a href="/configMqtt"><button type="button">MQTT</button></a>
  <p>
  <a href="/"><button type="button">Home</button></a>
</body>
</html>
)rawliteral";

static const char PROGMEM CONFIGUREDIV_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
  <title>DeviceName Configuration</title>
  <style>
    "body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
  </style>
</head>
<body>
  <h3>DeviceName Configuration</h3>
  <form method='POST' action='/configDiv' enctype='multipart/form-data'>
    <label>DeviceName AP name</label>
    <input type='text' name='apname' value="%s" size="32" maxlength="32"><br>
    <label>DNS name</label>
    <input type='text' name='mdnsname' value="%s" size="32" maxlength="32"><br>
    <label>Updater password:</label><br>
    <label>old user</label>
    <input type='text' name='olduser' value="%s" size="20" maxlength="20"><br>
    <label>old password</label>
    <input type='text' name='oldpassword' value="%s" size="20" maxlength="20"><br>
    <label>new user</label>
    <input type='text' name='newuser' size="20" maxlength="20"><br>
    <label>new password</label>
    <input type='text' name='newpassword' size="20" maxlength="20"><br>
    <p><input type='submit' value='Save changes'>
  </form>
  <p><a href="/configDevice"><button type="button">Cancel</button></a><a href="/configReset"><button type="button">Factory Reset</button></a>
</body>
</html>
)rawliteral";

static const char PROGMEM CONFIGUREMQTT_HTML[] = R"rawliteral(
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
  <h3>DeviceName MQTT Configuration</h3>
  <form method='POST' action='/configMqtt' enctype='multipart/form-data'>
    <label>MQTT broker</label>
    <input type='text' name='mqttbroker' value="%s" size="32" maxlength="32"><br>
    <label>MQTT client name</label>
    <input type='text' name='mqttclientname' value="%s" size="32" maxlength="32"><br>
    <label>MQTT client user</label>
    <input type='text' name='mqttclientuser' value="%s" size="32" maxlength="20"><br>
    <label>MQTT client password</label>
    <input type='text' name='mqttclientpassword' value="%s" size="32" maxlength="20"><br>
    <label>MQTT client Tx Topic</label>
    <input type='text' name='mqttclienttxtopic' value="%s" size="32" maxlength="20"><br>
    <label>MQTT client Rx Topic</label>
    <input type='text' name='mqttclientrxtopic' value="%s" size="32" maxlength="20"><br>    
    <label>note: if you have seen %s and changed MQTT broker then set RFM69 encrypt key again!</label>
    <p><input type='submit' value='Save changes'>
  </form>
  <p><a href="/configDevice"><button type="button">Cancel</button></a>
</body>
</html>
)rawliteral";

#ifndef userpage_existing 
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
      <h3>No Userpage defined or #define userpage_existing not set!</h3>
      <a href="/"><button type="button">Home</button></a>
    </body>
    </html>
    )rawliteral";
#endif

#include <WebSocketsServer.h>     //https://github.com/Links2004/arduinoWebSockets
#include <Hash.h>
ESP8266WebServer webServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, int type, uint8_t * payload, size_t length)
{
    Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
    switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);

      // send data to all connected clients
      //webSocket.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      //hexdump(payload, length);

      // echo data back to browser
      //webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
    }
}

void handleRoot()
{
    Serial.print("Free heap="); Serial.println(ESP.getFreeHeap());
    
    webServer.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webServer.uri();
    message += "\nMethod: ";
    message += (webServer.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += webServer.args();
    message += "\n";
    for (uint8_t i=0; i<webServer.args(); i++){
        message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }
    webServer.send(404, "text/plain", message);
}

void handleconfigureDevice()
{
    webServer.send_P(200, "text/html", CONFIGURE_HTML);
}

// Reset global config back to factory defaults
void handleconfigureReset()
{
    pGC->checksum++;
    EEPROM.commit();
    ESP.reset();
    delay(1000);
}

void handleconfigureDiv()
{
    size_t formFinal_len = strlen_P(CONFIGUREDIV_HTML) + sizeof(*pGC);
    char *formFinal = (char *)malloc(formFinal_len);
    if (formFinal == NULL) {
        Serial.println("formFinal malloc failed");
        return;
    }
    #if showKeysInWeb == true
      snprintf_P(formFinal, formFinal_len, CONFIGUREDIV_HTML,
          pGC->apname, pGC->mdnsname, pGC->updateUser, pGC->updatePassword
          );
    #else
      snprintf_P(formFinal, formFinal_len, CONFIGUREDIV_HTML,
          pGC->apname, pGC->mdnsname, HiddenString, HiddenString
          );
    #endif
    webServer.send(200, "text/html", formFinal);
    free(formFinal);
}

void handleconfigureDivWrite()
{
    bool commit_required = false;
    bool oldPasswordSeen = false;
    bool oldUserSeen = false;
    String argi, argNamei;

    for (uint8_t i=0; i<webServer.args(); i++) {
        Serial.print(webServer.argName(i));
        Serial.print('=');
        Serial.println(webServer.arg(i));
        argi = webServer.arg(i);
        argNamei = webServer.argName(i);
        if (argNamei == "apname") {
          const char *apname = argi.c_str();
            if (strcmp(apname, pGC->apname) != 0) {
                commit_required = true;
                strcpy(pGC->apname, apname);
            }
        }
        else if (argNamei == "olduser") {
            const char *olduser = argi.c_str();
            if (strcmp(olduser, HiddenString) != 0){
                if (strcmp(olduser, pGC->updateUser) == 0) {
                    oldUserSeen = true;
                }
            }
        }
        else if (argNamei == "oldpassword") {
            const char *oldpw = argi.c_str();
            if (strcmp(oldpw, HiddenString) != 0){
                if (strcmp(oldpw, pGC->updatePassword) == 0) {
                    oldPasswordSeen = true;
                }
            }
        }
        else if (argNamei == "newuser") {
          const char *newusr = argi.c_str();
            if ((strcmp(newusr, HiddenString) != 0) && oldPasswordSeen && oldUserSeen){
                if (strcmp(newusr, pGC->updatePassword) != 0) {
                    commit_required = true;
                    strcpy(pGC->updateUser, newusr);
                    Serial.println("user changed");
                }
            }
        }
        else if (argNamei == "newpassword") {
          const char *newpw = argi.c_str();
            if ((strcmp(newpw, HiddenString) != 0) && oldPasswordSeen && oldUserSeen){
                if (strcmp(newpw, pGC->updatePassword) != 0) {
                    commit_required = true;
                    strcpy(pGC->updatePassword, newpw);
                    Serial.println("password changed");
                }
            }
        }
        else if (argNamei == "mdnsname") {
            const char *mdns = argi.c_str();
            if (strcmp(mdns, pGC->mdnsname) != 0) {
                commit_required = true;
                strcpy(pGC->mdnsname, mdns);
            }
        }
    }
    handleRoot();
    if (commit_required) {
        pGC->checksum = gc_checksum();
        EEPROM.commit();
        ESP.reset();
        delay(1000);
    }
}

void handleconfigureMqtt()
{
    size_t formFinal_len = strlen_P(CONFIGUREMQTT_HTML) + sizeof(*pGC);
    char *formFinal = (char *)malloc(formFinal_len);
    if (formFinal == NULL) {}
    #if showKeysInWeb == true
        snprintf_P(formFinal, formFinal_len, CONFIGUREMQTT_HTML,
        pGC->mqttbroker, pGC->mqttclientname, pGC->mqttUser, pGC->mqttPassword, pGC->tx_topic, pGC->rx_topic, HiddenString
        );
    #else
        snprintf_P(formFinal, formFinal_len, CONFIGUREMQTT_HTML,
        HiddenString, pGC->mqttclientname,  HiddenString, HiddenString, pGC->tx_topic, pGC->rx_topic, HiddenString
        );
    #endif
    webServer.send(200, "text/html", formFinal);
    free(formFinal);
}

void handleconfigureMqttWrite()
{
    bool commit_required = false;
    String argi, argNamei;

    for (uint8_t i=0; i<webServer.args(); i++) {
        Serial.print(webServer.argName(i));
        Serial.print('=');
        Serial.println(webServer.arg(i));
        argi = webServer.arg(i);
        argNamei = webServer.argName(i);
        if (argNamei == "mqttbroker") {
            const char *broker = argi.c_str();
            if (strcmp(broker, HiddenString) != 0){
                if (strcmp(broker, pGC->mqttbroker) != 0) {
                    commit_required = true;
                    strcpy(pGC->mqttbroker, broker);
                    #ifdef MQTTBrokerChanged_existing
                        MQTTBrokerChanged();
                    #endif
                }
            }
        }
        else if (argNamei == "mqttclientname") {
            const char *client = argi.c_str();
            if (strcmp(client, pGC->mqttclientname) != 0) {
                commit_required = true;
                strcpy(pGC->mqttclientname, client);
            }
        }
        else if (argNamei == "mqttclientpassword"){
            const char *pw = argi.c_str();
            if (strcmp(pw, HiddenString) != 0){
                if (strcmp(pw, pGC->mqttPassword) != 0){
                    commit_required = true;
                    strcpy(pGC->mqttPassword, pw);
                }
            }
        }
        else if (argNamei == "mqttclientuser"){
            const char *user = argi.c_str();
            if (strcmp(user, HiddenString) != 0){
                if (strcmp(user, pGC->mqttUser) != 0){
                    commit_required = true;
                    strcpy(pGC->mqttUser, user);
                }
            }
        }
        else if (argNamei == "mqttclienttxtopic"){
            const char *txtopic = argi.c_str();

            if (strcmp(txtopic, pGC->tx_topic) != 0){
                commit_required = true;
                strcpy(pGC->tx_topic, txtopic);
            }

        }
        else if (argNamei == "mqttclientrxtopic"){
            const char *rxtopic = argi.c_str();

            if (strcmp(rxtopic, pGC->rx_topic) != 0){
                commit_required = true;
                strcpy(pGC->rx_topic, rxtopic);
            }

        }
    }

    handleRoot();
    if (commit_required) {
        pGC->checksum = gc_checksum();
        EEPROM.commit();
        ESP.reset();
        delay(1000);
    }
}

#ifndef userpage_existing
    void handleconfigureUserWrite(){
        handleRoot();
    }
    void handleconfigureUser(){
        webServer.send_P(200, "text/html", USERPAGE_HTML);
    }
#else
    void handleconfigureUserWrite();
    void handleconfigureUser();
#endif

void webServersend_P(const char* html){
    webServer.send_P(200, "text/html", html);
}

void websock_setup(void) {
    webServer.on("/", handleRoot);
    webServer.on("/configDevice", HTTP_GET, handleconfigureDevice); //startpage
    webServer.on("/configMqtt", HTTP_GET, handleconfigureMqtt);
    webServer.on("/configMqtt", HTTP_POST, handleconfigureMqttWrite);
    webServer.on("/configDiv", HTTP_GET, handleconfigureDiv);
    webServer.on("/configDiv", HTTP_POST, handleconfigureDivWrite);
    webServer.on("/configReset", HTTP_GET, handleconfigureReset);
    webServer.on("/configUserPage", HTTP_GET, handleconfigureUser);
    webServer.on("/configUserPage", HTTP_POST, handleconfigureUserWrite);
    webServer.onNotFound(handleNotFound);
    webServer.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

// ^^^^^^^^^ ESP8266 web sockets ^^^^^^^^^^^


// vvvvvvvvv ESP8266 Web OTA Updater vvvvvvvvvvv
#include "ESP8266HTTPUpdateServer.h"
ESP8266HTTPUpdateServer httpUpdater;

void ota_setup() {
    httpUpdater.setup(&webServer, "/updater", pGC->updateUser, pGC->updatePassword);//sw
}

// ^^^^^^^^^ ESP8266 Web OTA Updater ^^^^^^^^^^^



// vvvvvvvvv MQTT vvvvvvvvvvv
// *** Be sure to modify PubSubClient.h ***
//
// Be sure to increase the MQTT maximum packet size by modifying
// PubSubClient.h. Add the following line to the top of PubSubClient.h.
// Failure to make this modification means no MQTT messages will be
// published.
// #define MQTT_MAX_PACKET_SIZE 256




void mqtt_setup() {
    mqttClient.setServer(pGC->mqttbroker, 1883);
    mqttClient.setCallback(callback);
}
// If a UserSubscribe Funktion is declared we call it in the reconnect funktion


void reconnect() {
    
    static unsigned long timeLastConn = 0;
    
    if (strncmp(pGC->mqttbroker, "none", 4) != 0){
    // connect after 50s again
        if ((!timeLastConn) || ((timeLastConn + 100000) < millis())){
            Serial.print("Attempting MQTT connection...");
            // Attempt to connect
            if (mqttClient.connect(pGC->mqttclientname, pGC->mqttUser, pGC->mqttPassword)) {
                delay(1000);
                if (mqttClient.connected()) {
                    Serial.println("connected");
                    // Once connected, publish an announcement...
                    mqttClient.publish("DeviceName_Info", "connected");
                    // ... and resubscribe
                    #ifdef usersubscribe_existing
                        UserSubscribe();
                    #endif
                    delay(1000);
                    if (strlen(pGC->rx_topic)){
                        mqttClient.subscribe(pGC->rx_topic);
                    }
                    Serial.printf("subscribed to topic [%s]\r\n", pGC->rx_topic);
                } else {
                    Serial.println("not connected");
                }
            } else {
                Serial.print("failed, rc=");
                Serial.print(mqttClient.state());
                Serial.println(" try again in 100s");
            }
            timeLastConn = millis();
        }
    }
}

void mqtt_loop() {
    if (!mqttClient.connected()) {
        reconnect();
    }
    
    mqttClient.loop();
}

void mqttpublishJSON(const char* key, const char* value){
    char payload[200] ="\"";
    strncat(payload, key, 100);
    strncat(payload, "\":\"", 10);
    strncat(payload, value, 100);
    strncat(payload, "\"", 10);
    mqttClient.publish(pGC->tx_topic, payload);
}

void mqttpublishTXTopic(const char* payload){
     mqttClient.publish(pGC->tx_topic, payload);
}

void mqttpublish(const char* topic, const char* payload){
     mqttClient.publish(topic, payload);
}

void mqttpublish(const char* topic, const char* payload, boolean retained){
     mqttClient.publish(topic, payload, retained);
}

boolean mqttConnected(){
    return mqttClient.connected();
}

void mqttSubscribe(const char* topic){
    mqttClient.subscribe(topic);
}

void mqttUnsubscribe(const char* topic){
    mqttClient.unsubscribe(topic);
}

// ^^^^^^^^^ MQTT ^^^^^^^^^^^


// Start webserver, ota, mqtt ect complete

void setup_server(){
    
    Serial.begin(SERIAL_BAUD);
    Serial.println("eeprom_setup");
    eeprom_setup();
    Serial.println("wifi_setup");
    wifi_setup();
    Serial.println("mdns_setup");
    mdns_setup();
    Serial.println("mqtt_setup");
    mqtt_setup();
    Serial.println("ota_setup");
    ota_setup();
    Serial.println("websock_setup");
    websock_setup();
    Serial.println("webserver_setup_finished");
    
}

void loop_server(){
  
    mqtt_loop();
    //Serial.println("webSocket");
    webSocket.loop();
    //Serial.println("webServer");
    webServer.handleClient();
}

