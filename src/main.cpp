#include <SPI.h>              // include libraries
#include <WiFi.h>
#include "led-display.h"
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "wifiCommunication.h"
#include "mqtt.h"
#include "engine.h"


char ssid[23];
uint8_t macAddr[6];
char sMacAddr[18];

void sendMessage(const char *topic, const char *outgoing);
void publish_alive();

SPIClass spi(VSPI);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Wire.begin(SDA, SCL);

   // Get deviceId
  snprintf(ssid, 23, "MCUDEVICE-%llX", ESP.getEfuseMac());
  WiFi.macAddress(macAddr);   // The MAC address is stored in the macAddr array.
  snprintf(sMacAddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  Serial.println(ssid);
  Serial.println(sMacAddr);

  wifi_setup();
  wifi_enable();

  mqtt_setup();
  mqtt_loop();

  publish_alive();

  delay(2000);

  setupEngine();
}

void loop()
{

  mqtt_loop();
  engine_loop();

}

void publish_alive() {

  // maximum message length 128 Byte
  String payload = "";
  payload += "{\"device\":";
  payload += "\"";
  payload += ssid;
  payload += "\"";
  payload += ",\"type\":";
  payload += "\"iamalive\"";
  payload += ",\"mac\":";
  payload += "\"";
  payload += sMacAddr;
  payload += "\"";
  payload += "}";
  sendMessage(mqttAliveTopic, payload.c_str());
}

void sendMessage(const char *topic, const char *outgoing) {
  publishMQTTMessage(topic, outgoing);
  Serial.println(outgoing);
}