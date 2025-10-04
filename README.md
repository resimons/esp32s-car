# Connect a BME280 to a ESP 32S with LoRa SX1278 Module RA-02

The ESP will read the data from the sensor and puts it on the LCD display.

## Pin layout ESP32

![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/esp32-esp-32s-nodemcu-pinout.jpg)
![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/bme280.jpeg)

## Wiring

## Where to buy

[ESP-32S](https://elektronicavoorjou.nl/product/esp32-development-board-wifi-bluetooth)
[BME280](https://www.tinytronics.nl/nl/sensoren/lucht/druk/bme280-digitale-barometer-druk-en-vochtigheid-sensor-module)

## How to connect BME280 and LCD display to ESP32
COMPONENT | COLOUR | ESP
------------ | ---------- | -------------
BME280 VCC | RED | 3.3V
BME280 GND | BLACK | GND
BME280 SCL | GREEN | GPIO22
BME280 SDA | YELLOW | GPIO21

# Pinlayout LoRa SX1278 Module ra-02

![image](https://images.tcdn.com.br/img/img_prod/557243/sx1278_lora_433mhz_ra_02_breakout_board_10km_959_1_20191128221303.png)

## Wiring LoRa SX1278 module with ESP32
SX1278 | COLOR | ESP32
-------- |-| ----------
3.3V | RED | 3.3V
GND | BLACK | GND
SCK | ORANGE | GPIO18
MOSI | WHITE | GPIO23
DIO0 | BROWN | GPIO4
NSS | PINK | GPIO5
MISO | GREY | GPIO19
RST | YELLOW | GPIO2

[Wiring and more](https://www.circuitstate.com/tutorials/interfacing-ra-01-ra-02-sx1278-lora-modules-with-esp32-using-arduino/)
[Sample code](https://how2electronics.com/esp32-lora-sx1278-76-transmitter-receiver/)
