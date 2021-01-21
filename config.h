///////////////////////////////////
/////////////// CONFIG
///////////////////////////////////
//*********** WiFi CONFIG *****************
const char *ssid = "your_ssid";
const char *password = "your_pass";
//*********** MQTT CONFIG *****************
const char *mqtt_server = "broker.mqtt.com";
int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_pass = "";
const char *client_id = "esp_estudio";
const char *root_topic_subscribe = "pub/casa/estudio/#";//Suscripcion a todos los topicos apartir del #
const char *root_topic_publish = "pub/casa/estudio";
//*********** OTA CONFIG ******************
const char *Hostname_ota = "esp-estudio";
const char *password_ota = "change";
uint16_t port_ota = 8266;
