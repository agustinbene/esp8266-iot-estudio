/*/Label GPIO     Input         Output notes
   D1    GPIO5    ok            ok            often used as SDA (I2C)
   D2    GPIO4    ok            ok            often used as SDA (I2C)
   D5    GPIO14   ok            ok            SPI (SCLK)
   D6    GPIO12   ok            ok            SPI (MISO)
   D7    GPIO13   ok            ok            SPI (MOSI)
   D8    GPIO15   Pull_dwn      OK            SPI (CS)Boot fails if pulled HIGH
   Rx    GPIO3    OK            X             HIGH at boot
   Tx    GPIO1    X             OK            HIGH at boot. debug output at boot, boot fails if pulled LOW
   A0    ADC0     Analog        X             -
   D3    GPIO0    Pull_up       ok            connected to FLASH button, boot fails if pulled LOW
   D4    GPIO2    Pull_up       ok            HIGH at boot,connected to on-board LED, boot fails if pulled LOW
   D0    GPIO16   no interrupt  no pwmor i2c  HIGH at boot
*/
#include <Arduino.h>
#include "gicom_IoT.h"
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <secrets.h>
///////////////////////////////////
/////////////// CONFIG
///////////////////////////////////
//*********** LUZ ************************
#define tecla_pin_1 14
#define tecla_pin_2 12
#define PIN_RELAY_1 5
#define PIN_RELAY_2 4
long tTecla = 0;
int flagtecla1, flagtecla2;
//*********** DHT *************************
#include "DHT.h"
#define power_dht 16 //d0
#define dht_pin 2 //d4
#define DHTTYPE DHT22
DHT dht(dht_pin, DHTTYPE);
float t = -1, t_old = -1, h = -1, h_old = -1, hic_old = -1, hic = -1;
//*********** Movimiento **********************
//#define mov_pin 14
int mov = -1, mov_old = -1;

//*********** GLOBALES ********************
gicom_iot iot; //Declaracion de la clase gicom_iot como "iot"
char msg[250]; //Msg de llegada de mqtt
long count = 0;
unsigned long previousMillis = 0;
const int ledPin =  LED_BUILTIN;
float rssi = 0;
long tiempo1 = 0;
void tecla();




///////////////////////////////////
//     SETUP
///////////////////////////////////
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  iot.wifi_config(ssid, password);                //Configuracion del wifi con reconeccion automatica
  iot.mqtt(mqtt_server, mqtt_port);               //Configuracion del servidor mqtt
  iot.ota(Hostname_ota, password_ota, port_ota);  //Configuracion del ota !while interno hasta que se conecte el wifi

  pinMode(power_dht, OUTPUT);
  digitalWrite(power_dht, LOW);
  delay(1500);
  digitalWrite(power_dht, HIGH);
  dht.begin();
  //pinMode(mov_pin, INPUT);
  pinMode(tecla_pin_1, INPUT);
  pinMode(tecla_pin_2, INPUT);
  flagtecla1 = digitalRead(tecla_pin_1);
  flagtecla2 = digitalRead(tecla_pin_2);
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);

}


///////////////////////////////////
//       LOOP
///////////////////////////////////
void loop() {

  iot.loop_ota(); //Funcion necesaria para OTA
  iot.loop_mqtt(mqtt_user, mqtt_pass, root_topic_subscribe, client_id); //Funcion necesaria para mqtt


  if (millis() - previousMillis >= 5000) { //Ejecuta cada 5 seg.
    perifericos();
  }

  if (millis() - tTecla > 150) tecla();


}



//*********** PERIFERICOS ***************************************************************->PERIFERICOS
void perifericos() {
  previousMillis = millis();
  if (iot.mqtt_estado()) {

    if (WiFi.RSSI() != rssi) {
      rssi = WiFi.RSSI();
      char rssih[6];
      dtostrf(rssi, 6, 0, rssih);

      iot.send_mqtt(rssih, root_topic_publish, "rssi");
    }

    t = dht.readTemperature();
    Serial.print("temp: ");
    Serial.println(t);
    if (t != t_old) {
      t_old = t;
      char th[4];
      dtostrf(t, 4, 1, th);
      iot.send_mqtt(th, root_topic_publish, "temperatura");
    }
    h = dht.readHumidity();
    if (h != h_old && h < 101) {
      h_old = h;
      char hh[4];
      dtostrf(h, 5, 1, hh);
      iot.send_mqtt(hh, root_topic_publish, "humedad");
    }

    float hic = dht.computeHeatIndex(t, h, false);
    if (hic != hic_old) {
      hic_old = hic;
      char hich[5];
      dtostrf(hic, 5, 0, hich);
      iot.send_mqtt(hich, root_topic_publish, "hic");
    }
  }
}

//*********** CALLBACK ***************************************************************->CALLBACK

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //iot.comparar -> Funcion que contatena el topico raiz con el tercer parametro y lo compara con el topico de entrada,el primer parametro
  if (iot.comparar(topic, root_topic_subscribe, "luz1") == 0) {
    if ((char)payload[0] == '0') {
      digitalWrite(PIN_RELAY_1, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(PIN_RELAY_1, LOW);  // Turn the LED off by making the voltage HIGH
    }
  }
  if (iot.comparar(topic, root_topic_subscribe, "luz2") == 0) {
    if ((char)payload[0] == '0') {
      digitalWrite(PIN_RELAY_2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    } else {
      digitalWrite(PIN_RELAY_2, LOW);  // Turn the LED off by making the voltage HIGH
    }
  }
  if (iot.comparar(topic, root_topic_subscribe, "reset") == 0) {
    if ((char)payload[0] == '1') {
      //iot.send_mqtt("0", root_topic_publish, "reset");
      //delay(100);
      Serial.print("reset");
      ESP.restart();
    }
  }
  if (iot.comparar(topic, root_topic_subscribe, "slider") == 0) {
    payload[length] = '\0';
    int pwm = atoi((char *)payload);
    analogWrite(4, pwm);
  }


  
}

void tecla() {

  if (flagtecla1 != digitalRead(tecla_pin_1)) {
    Serial.println("Tecla");
    flagtecla1 = digitalRead(tecla_pin_1);
    if (digitalRead(PIN_RELAY_1)) {
      digitalWrite(PIN_RELAY_1, LOW); //pin rele high rele apagado, lampara apagada
      if (iot.mqtt_estado()) {
        iot.send_mqtt("1", root_topic_publish, "luz1");
      }
    }
    else {
      digitalWrite(PIN_RELAY_1, HIGH); //pin rele low rele encendido, lampara encendida
      if (iot.mqtt_estado()) {
        iot.send_mqtt("0", root_topic_publish, "luz1");
      }
    }
  }
  if (flagtecla2 != digitalRead(tecla_pin_2)) {
    Serial.println("Tecla");
    flagtecla2 = digitalRead(tecla_pin_2);
    if (digitalRead(PIN_RELAY_2)) {
      digitalWrite(PIN_RELAY_2, LOW); //pin rele high rele apagado, lampara apagada
      if (iot.mqtt_estado()) {
        iot.send_mqtt("1", root_topic_publish, "luz2");
      }
    }
    else {
      digitalWrite(PIN_RELAY_2, HIGH); //pin rele low rele encendido, lampara encendida
      if (iot.mqtt_estado()) {
        iot.send_mqtt("0", root_topic_publish, "luz2");
      }
    }
  }
  tTecla = millis();
}
