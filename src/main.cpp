#include <SPI.h>              // include libraries
#include <WiFi.h>
#include "led-display.h"
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include "ir-commands.h"


char ssid[23];
uint8_t macAddr[6];
char sMacAddr[18];

void sendMessage(String outgoing);
void publish_alive();

SPIClass spi(VSPI);
LiquidCrystal_I2C lcd(0x27 /* I2C address */, 16 /* number of digits in a row */, 2 /* number of rows */);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Wire.begin(SDA, SCL);

  IrReceiver.begin(GPIO_NUM_15, ENABLE_LED_FEEDBACK);  

  lcd.init();                      // initialize the lcd 
  lcd.setBacklight(HIGH); //Set Back light turn On
  lcd.setCursor(0,0); // Move cursor to 0

   // Get deviceId
  snprintf(ssid, 23, "MCUDEVICE-%llX", ESP.getEfuseMac());
  WiFi.macAddress(macAddr);   // The MAC address is stored in the macAddr array.
  snprintf(sMacAddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  Serial.println(ssid);
  Serial.println(sMacAddr);

  lcd.clear();
  lcd.setCursor(0,0); // Move cursor to 0
  delay(2500);

  publish_alive();

  // testLedSegments();

}

void sendMessage(String outgoing);

void loop()
{
  if (IrReceiver.decode()) {
      uint16_t command = IrReceiver.decodedIRData.command;
      Serial.println(command);
      delay(100);  // wait a bit
      IrReceiver.resume();

      switch(command) {
        case COMMAND_LEFT:
          lcd.print("LEFT");
          break;
        case COMMAND_RIGHT:
          lcd.print("RIGHT");
          break;
        case COMMAND_FORWARD:
          lcd.print("FORWARD");
          break;
        case COMMAND_BACKWARD:
          lcd.print("BACKWARD");
          break;
      }

  }
    // // The sensor should be read every 60 seconds when in Forced mode, according to the datasheet and Adafruit library
    // showSpinner(30000);
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
  sendMessage(payload);
}

void sendMessage(String outgoing) {
}