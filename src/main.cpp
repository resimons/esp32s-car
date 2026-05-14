#include "config.h"
#include "engine.h"
#include <BLEGamepadClient.h>

char ssid[23];
uint8_t macAddr[6];
char sMacAddr[18];

BLEAutoScan *pAutoScan = BLEGamepadClient::getAutoScan();

XboxController controller;
void onScanStarted() {
  Serial.println("scan started");
}
void onScanStopped() {
  Serial.println("scan stopped");
}
void onConnecting(XboxController &ctrl) {
  Serial.println("connecting");
}
void onConnectionFailed(XboxController &ctrl) {
  Serial.println("connection failed");
}
void onConnected(XboxController &ctrl) {
  Serial.println("connected");
}
void onDisconnected(XboxController &ctrl) {
  Serial.println("disconnected");
}
void onValueChanged(XboxControlsState &s) {
  Serial.printf("lstick: %.2f,%.2f, rstick: %.2f,%.2f\n",
    s.leftStickX, s.leftStickY, s.rightStickX, s.rightStickY);
}
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pAutoScan->onScanStarted(onScanStarted);
  pAutoScan->onScanStopped(onScanStopped);

  controller.begin();
  controller.onConnecting(onConnecting);
  controller.onConnectionFailed(onConnectionFailed);
  controller.onConnected(onConnected);
  controller.onDisconnected(onDisconnected);
  controller.onValueChanged(onValueChanged);

  delay(2000);

  setupEngine();


}

void loop()
{

  engine_loop();
  delay(1000);
}
