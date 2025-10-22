#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <SPIFFS.h>

#include "config.h"
#include "wifiCommunication.h"
#include "mqtt.h"

// For TLS checkout https://stackoverflow.com/questions/66204709/how-to-connect-an-esp32-to-mqtt-server-with-public-ip-and-ssl

unsigned long reconnectInterval = 5000;
// in order to do reconnect immediately ...
unsigned long lastReconnectAttempt = millis() - reconnectInterval - 1;

WiFiClientSecure net;
MQTTClient client;

bool checkMQTTconnection();

char clientId[23];


void mqtt_setup() {

  // Get deviceId
  snprintf(clientId, 23, "MCUDEVICE-%llX", ESP.getEfuseMac());

  net.setCACert(rootCABuff);
	net.setCertificate(certificateBuff);
	net.setPrivateKey(privateKeyBuff);

	client.begin(mqtt_server, mqtt_server_port, net);

  	while (!client.connect(clientId)) {
		Serial.print(".");
		delay(100);
	}

  Serial.printf("  Successfully connected to MQTT broker\r\n");
}

void mqtt_loop(){
  if (!client.connected()) {
    unsigned long currentMillis = millis();
    if ((currentMillis - lastReconnectAttempt) > reconnectInterval) {
      lastReconnectAttempt = currentMillis;
      // Attempt to reconnect
      checkMQTTconnection();
    }
  }  

  if (client.connected()) {
    client.loop();
  }
}

bool checkMQTTconnection() {
  if (wifiIsDisabled) return false;

  if (WiFi.isConnected()) {
    if (client.connected()) {
      return true;
    } else {
      // try to connect to mqtt server
      if (client.connect(clientId)) {
        Serial.printf("  Successfully connected to MQTT broker\r\n");
      } else {
        Serial.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");
      }
      return client.connected();
    }
  } else {
    Serial.printf("  No connection to MQTT server, because WiFi ist not connected.\r\n");
    return false;
  }  
}

void publishMQTTMessage( const char *topic, const char *payload){
  if (wifiIsDisabled) return;

  if (checkMQTTconnection()) {
      
    if (!client.publish(topic, payload)) {
      Serial.printf("Publish failed\r\n");
    }
  } else {
    Serial.printf("  Cannot publish mqtt message, because checkMQTTconnection failed (WiFi or mqtt is not connected)\r\n");
  }
}