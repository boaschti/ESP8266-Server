
#include <arduino.h>

void setup_server();

void loop_server();

void handleRoot();

void webServersend_P(const char* html);

// publishes in JSON Format to TX Topic
void mqttpublishJSON(const char* key, const char* value);

// pubishes with editable tx topic
void mqttpublishTXTopic(const char* payload);

// pubishes with editable userspecific topic
void mqttpublish(const char* topic, const char* payload);

void mqttpublish(const char* topic, const char* payload, boolean retained);

void mqttSubscribeStateTopic();

void mqttUnsubscribeStateTopic();

void mqttSubscribeSetTopic();

void mqttUnsubscribeSetTopic();

void mqttSubscribe(const char* topic);

void mqttUnsubscribe(const char* topic);

boolean mqttConnected();

uint8_t eeprom_write_byte(uint16_t address, uint8_t value);

uint8_t eeprom_read_byte(uint16_t address);