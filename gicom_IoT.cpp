#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "gicom_iot.h"
#include "Arduino.h"
#include <PubSubClient.h>


WiFiClient espClient;
PubSubClient client(espClient);
long lastReconnectAttempt = 0;
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
char* callback(char* topic, byte * payload, unsigned int length);

void gicom_iot::wifi_config(const char* ssid, const char* password) {
  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP & event) {
    Serial.print("Station connected, IP: ");
    Serial.println(WiFi.localIP());
  });
  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected & event)
  {
    Serial.println("Station disconnected");
  });
  Serial.printf("Connecting to %s ...\n", ssid);
  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid, password);
}

void gicom_iot::mqtt(const char* mqtt_server, int mqtt_port) {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void gicom_iot::ota(const char* Hostname_ota, const char* password_ota, uint16_t port) {

  while (WiFi.status() != WL_CONNECTED)delay(0);//Espera hasta que se obtenga ip y se conecte al wifi

  // Port defaults to 8266
  ArduinoOTA.setPort(port);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(Hostname_ota);

  // No authentication by default
  ArduinoOTA.setPassword(password_ota);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    ESP.restart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void gicom_iot::loop_ota() {
  ArduinoOTA.handle();
}



//*****************************
//***    CONEXION MQTT      ***
//*****************************
void gicom_iot::loop_mqtt(const char *mqtt_user, const char *mqtt_pass, const char *root_topic_subscribe, const char *clientId) {
  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    if (millis() - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = millis();
      Serial.print("Intentando conexión Mqtt...");
      Serial.print(String(mqtt_user));
      Serial.print("...");
      Serial.print(String(mqtt_pass));
      // Creamos un cliente ID
      //String(clientId) += String(random(0xffff), HEX);
      Serial.print("clientId: ");
      Serial.println(clientId);
      // Intentamos conectar
      if (client.connect(clientId, mqtt_user, mqtt_pass)) {
        Serial.println("Conectado!");
        // Nos suscribimos
        if (client.subscribe(root_topic_subscribe)) {
          Serial.println("Suscripcion ok");
        } else {
          Serial.println("fallo Suscripciión");
        }
      } else {
        Serial.print("falló :( con error -> ");
        Serial.print(client.state());
        Serial.println(" Intentamos de nuevo en 5 segundos");
      }
    }
    if (client.connected()) {
      lastReconnectAttempt = 0;
    }
  } else {
    // Client connec
    client.loop();
  }
}


boolean gicom_iot::mqtt_estado() {
  return client.connected();
}

void gicom_iot::send_mqtt(char msg[25],const char root_topic_publish[100], char end_topic[25]) {
  char send_topic_char[100];
  String send_topic = String(root_topic_publish) + "/" + String(end_topic);
  send_topic.toCharArray(send_topic_char, 100);
  
  client.publish(send_topic_char, msg, true);
}


//iot.comparar -> Funcion que contatena el topico raiz con el tercer parametro y lo compara con el topico de entrada,el primer parametro
boolean gicom_iot::comparar(char* topic, const char* root_topic_subscribe, String x) {
  String topico; //topico_raiz+topico_especifico "pub/node/bebederos/3/" + "luz"
  char char_topico[60];
  topico = String(root_topic_subscribe);
  topico.remove(topico.length() - 1, 1);
  topico = topico + x;
  topico.toCharArray(char_topico, 50);
  return strcmp(topic, char_topico);
}
