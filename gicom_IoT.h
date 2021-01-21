#ifndef gicom_iot_h
#define gicom_iot_h
#include "Arduino.h"

class gicom_iot{

  public:
    void wifi_config(const char* ssid, const char* password);
    void ota(const char* Hostname_ota, const char* password_ota, uint16_t port);
    void mqtt(const char* mqtt_server, int mqtt_port);
    void loop_ota();
    void loop_mqtt(const char *mqtt_user, const char *mqtt_pass,const char *root_topic_subscribe,const char *clientId);
    boolean mqtt_estado();
    boolean comparar(char* topic,const char* root_topic_subscribe, String x);
    void send_mqtt(char msg[25],const char root_topic_publish[100], char end_topic[25]);
    void encender(int pin);
    void apagar(int pin);

  private:
};

#endif
