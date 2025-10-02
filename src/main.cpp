#include <SPI.h>              // include libraries
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <LoRa.h>
#include <Adafruit_BME280.h>

#define LORA_FREQ 433E6

#define SEALEVELPRESSURE_HPA (1013.25)

unsigned long previousMillisProbe = 0;
unsigned long intervalProbe = 4000;      // this is the internal update interval of the CO2 sensor
unsigned long previousMillisPublish = 0;
unsigned long intervalPublish = 60000;

char ssid[23];
uint8_t macAddr[6];
char sMacAddr[18];

void sendMessage(String outgoing);
void publish_alive();

SPIClass spi(VSPI);
Adafruit_BME280 bme;

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

  Serial.println("Looking for sensor");
  if (! bme.begin(BME280_ADDRESS_ALTERNATE)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("Sensor found");

  LoRa.setPins(SS, GPIO_NUM_2, GPIO_NUM_4);
  LoRa.setSPIFrequency (20000000);
  LoRa.setTxPower (20);
  if (!LoRa.begin(LORA_FREQ)) {
    delay (5000);
    ESP.restart();
  }

  Serial.println("Lora found");

  LoRa.setPreambleLength(8);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);

    // suggested rate is 1/60Hz (1m)
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
                    

  publish_alive();
}

void sendMessage(String outgoing);
void displayAndSendBmeValues();

void loop()
{

    bme.takeForcedMeasurement();
    
    displayAndSendBmeValues();

    // The sensor should be read every 60 seconds when in Forced mode, according to the datasheet and Adafruit library
    delay(60000);
}

void displayAndSendBmeValues() {

    String temp = String(bme.readTemperature());
    String pressure = String(bme.readPressure() / 100);
    String humidity = String(bme.readHumidity());
    String altitude = String(bme.readAltitude(SEALEVELPRESSURE_HPA));

    String payload = "";
    payload += "{\"temperature\":";
    payload += temp;
    payload += ",\"pressure\":";
    payload += pressure;
    payload += ",\"humidity\":";
    payload += humidity;
    payload += ",\"altitude\":";
    payload += altitude;
    payload += ",\"sensor\":";
    payload += "\"bme280\"";
    payload += ",\"device\":";
    payload += "\"";
    payload += ssid;
    payload += "\"";
    payload += ",\"mac\":";
    payload += "\"";
    payload += sMacAddr;
    payload += "\"";
    payload += "}";

    sendMessage(payload);
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
  LoRa.beginPacket();                   // start packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
}

