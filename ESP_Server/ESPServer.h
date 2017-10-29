  
#ifndef usersubscribe_existing
//#define usersubscribe_existing
#endif

#ifndef MQTTBrokerChanged_existing
//#define MQTTBrokerChanged_existing
#endif
    
#ifndef userpage_existing
//#define userpage_existing
#endif


void setup_server();

void loop_server();

void handleRoot();

void webServersend_P(const char* html);

void mqttpublish(const char* topic, const char* payload);

void mqttpublish(const char* topic, const char* payload, bool retained);
