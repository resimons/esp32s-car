#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "wifiCommunication.h"
#include "mqtt.h"
#include "engine.h"

unsigned long reconnectInterval = 5000;
unsigned long lastReconnectAttempt = millis() - reconnectInterval - 1;

WiFiClientSecure net;
MQTTClient client;

char clientId[23];

bool checkMQTTconnection();
void subscribeTopics();
void messageReceived(String &topic, String &payload);
void handleCommand(const String& command, int speed, int duration);

void mqtt_setup() {
  snprintf(clientId, 23, "MCUDEVICE-%llX", ESP.getEfuseMac());

  net.setCACert(rootCABuff);
  net.setCertificate(certificateBuff);
  net.setPrivateKey(privateKeyBuff);

  client.begin(mqtt_server, mqtt_server_port, net);
  client.onMessage(messageReceived);

  while (!client.connect(clientId)) {
    Serial.print(".");
    delay(100);
  }

  Serial.printf("  Successfully connected to MQTT broker\r\n");
  subscribeTopics();
}

void subscribeTopics() {
  client.subscribe(mqttCommandTopic);
  Serial.printf("  Subscribed to %s\r\n", mqttCommandTopic);
}

void mqtt_loop() {
  if (!client.connected()) {
    unsigned long currentMillis = millis();
    if ((currentMillis - lastReconnectAttempt) > reconnectInterval) {
      lastReconnectAttempt = currentMillis;
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
      if (client.connect(clientId)) {
        Serial.printf("  Successfully connected to MQTT broker\r\n");
        subscribeTopics();
      } else {
        Serial.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");
      }
      return client.connected();
    }
  } else {
    Serial.printf("  No connection to MQTT server, because WiFi is not connected.\r\n");
    return false;
  }
}

void publishMQTTMessage(const char *topic, const char *payload) {
  if (wifiIsDisabled) return;

  if (checkMQTTconnection()) {
    if (!client.publish(topic, payload)) {
      Serial.printf("  Publish failed\r\n");
    }
  } else {
    Serial.printf("  Cannot publish: WiFi or MQTT not connected\r\n");
  }
}

// ---------------------------------------------------------------------------
// Command interpreter
// ---------------------------------------------------------------------------

void handleCommand(const String& command, int speed, int duration) {
  if (command == "forward") {
    wheelLeftBackForward(speed);
    wheelLeftFrontForward(speed);
    wheelRightFrontForward(speed);
    wheelRightBackForward(speed);
  } else if (command == "backward") {
    wheelLeftBackBackward(speed);
    wheelLeftFrontBackward(speed);
    wheelRightFrontBackward(speed);
    wheelRightBackBackward(speed);
  } else if (command == "pivotLeft") {
    pivotLeft(speed);
  } else if (command == "pivotRight") {
    pivotRight(speed);
  } else if (command == "arcLeft") {
    arcLeft(speed / 2, speed);
  } else if (command == "arcRight") {
    arcRight(speed / 2, speed);
  } else if (command == "adjustSpeed") {
    adjustSpeed(speed);
    return;
  } else if (command == "reverseAll") {
    reverseAll();
    return;
  } else if (command == "stop") {
    stop();
    return;
  } else {
    Serial.printf("  Unknown command: %s\r\n", command.c_str());
    return;
  }

  if (duration > 0) scheduleStop(duration);
}

void messageReceived(String &topic, String &payload) {
  Serial.printf("  MQTT [%s]: %s\r\n", topic.c_str(), payload.c_str());

  if (topic != mqttCommandTopic) {
    Serial.printf("  Ignoring topic %s\r\n", topic.c_str());
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.printf("  JSON parse error: %s\r\n", error.c_str());
    return;
  }

  const char* device  = doc["device"]   | "";
  const char* command = doc["command"]  | "";
  int         speed   = doc["speed"]    | 0;
  int         duration = doc["duration"] | 0;

  // If device is specified, ignore commands not addressed to this device
  if (strlen(device) > 0 && strcmp(device, clientId) != 0) {
    Serial.printf("  Ignoring command for device %s\r\n", device);
    return;
  }

  if (strlen(command) == 0) {
    Serial.printf("  Missing 'command' field\r\n");
    return;
  }

  Serial.printf("  Executing: %s  speed=%d  duration=%d\r\n", command, speed, duration);
  handleCommand(String(command), speed, duration);
}
