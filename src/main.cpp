#include <SPI.h>              // include libraries
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <LoRa.h>
#include <Adafruit_BME280.h>

#include <TM1637Display.h>
#include <LiquidCrystal_I2C.h>

// Module connection pins (Digital Pins)
#define CLK GPIO_NUM_25
#define DIO GPIO_NUM_26

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   2000

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

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
void testSegmentDisplay();

SPIClass spi(VSPI);
Adafruit_BME280 bme;
TM1637Display display(CLK, DIO);
LiquidCrystal_I2C lcd(0x27 /* I2C address */, 16 /* number of digits in a row */, 2 /* number of rows */);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Wire.begin(SDA, SCL);

  lcd.init();                      // initialize the lcd 
  lcd.setBacklight(HIGH); //Set Back light turn On
  lcd.setCursor(0,0); // Move cursor to 0

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

  lcd.setCursor(0,0); // Move cursor to 0
  lcd.print("BME280 found");
  delay(2500);

  LoRa.setPins(SS, GPIO_NUM_2, GPIO_NUM_4);
  LoRa.setSPIFrequency (20000000);
  LoRa.setTxPower (20);
  if (!LoRa.begin(LORA_FREQ)) {
    delay (5000);
    ESP.restart();
  }

  lcd.clear();
  lcd.setCursor(0,0); // Move cursor to 0
  lcd.print("LoRa found");
  delay(2500);

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

  testSegmentDisplay();
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

    lcd.clear();
    lcd.setBacklight(HIGH); //Set Back light turn On
    lcd.setCursor(0,0); // Move cursor to 0
    lcd.print("Temp:  " + temp + " C");
    lcd.setCursor(0,1);
    lcd.print("Hum:   " + humidity + "%");
    delay(2500);
    lcd.setBacklight(LOW); // Set backlight to off
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

void testSegmentDisplay()
{
  int k;
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
  display.setBrightness(0x0f);

  // All segments on
  display.setSegments(data);
  delay(TEST_DELAY);

  // Selectively set different digits
  data[0] = display.encodeDigit(0);
  data[1] = display.encodeDigit(1);
  data[2] = display.encodeDigit(2);
  data[3] = display.encodeDigit(3);
  display.setSegments(data);
  delay(TEST_DELAY);

  /*
  for(k = 3; k >= 0; k--) {
	display.setSegments(data, 1, k);
	delay(TEST_DELAY);
	}
  */

  display.clear();
  display.setSegments(data+2, 2, 2);
  delay(TEST_DELAY);

  display.clear();
  display.setSegments(data+2, 2, 1);
  delay(TEST_DELAY);

  display.clear();
  display.setSegments(data+1, 3, 1);
  delay(TEST_DELAY);


  // Show decimal numbers with/without leading zeros
  display.showNumberDec(0, false); // Expect: ___0
  delay(TEST_DELAY);
  display.showNumberDec(0, true);  // Expect: 0000
  delay(TEST_DELAY);
	display.showNumberDec(1, false); // Expect: ___1
	delay(TEST_DELAY);
  display.showNumberDec(1, true);  // Expect: 0001
  delay(TEST_DELAY);
  display.showNumberDec(301, false); // Expect: _301
  delay(TEST_DELAY);
  display.showNumberDec(301, true); // Expect: 0301
  delay(TEST_DELAY);
  display.clear();
  display.showNumberDec(14, false, 2, 1); // Expect: _14_
  delay(TEST_DELAY);
  display.clear();
  display.showNumberDec(4, true, 2, 2);  // Expect: __04
  delay(TEST_DELAY);
  display.showNumberDec(-1, false);  // Expect: __-1
  delay(TEST_DELAY);
  display.showNumberDec(-12);        // Expect: _-12
  delay(TEST_DELAY);
  display.showNumberDec(-999);       // Expect: -999
  delay(TEST_DELAY);
  display.clear();
  display.showNumberDec(-5, false, 3, 0); // Expect: _-5_
  delay(TEST_DELAY);
  display.showNumberHexEx(0xf1af);        // Expect: f1Af
  delay(TEST_DELAY);
  display.showNumberHexEx(0x2c);          // Expect: __2C
  delay(TEST_DELAY);
  display.showNumberHexEx(0xd1, 0, true); // Expect: 00d1
  delay(TEST_DELAY);
  display.clear();
  display.showNumberHexEx(0xd1, 0, true, 2); // Expect: d1__
  delay(TEST_DELAY);
  
	// Run through all the dots
	for(k=0; k <= 4; k++) {
		display.showNumberDecEx(0, (0x80 >> k), true);
		delay(TEST_DELAY);
	}

  // Brightness Test
  for(k = 0; k < 4; k++)
	data[k] = 0xff;
  for(k = 0; k < 7; k++) {
    display.setBrightness(k);
    display.setSegments(data);
    delay(TEST_DELAY);
  }
  
  // On/Off test
  for(k = 0; k < 4; k++) {
    display.setBrightness(7, false);  // Turn off
    display.setSegments(data);
    delay(TEST_DELAY);
    display.setBrightness(7, true); // Turn on
    display.setSegments(data);
    delay(TEST_DELAY);  
  }

 
  // Done!
  display.setSegments(SEG_DONE);

  delay(10000);
}