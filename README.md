# Connect a BME280 to a ESP32-C3

The ESP will read the data from the sensor and puts it on the LCD display.

## Pin layout ESP32

![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/esp32-esp-32s-nodemcu-pinout.jpg)
![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/LCD-Module-Board.jpg)
![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/bme280.jpeg)

## Wiring

## Where to buy

[ESP32](https://www.hobbyelectronica.nl/product/esp32c3-development-board-zonder-serial-chip/)
[BME280](https://www.tinytronics.nl/nl/sensoren/lucht/druk/bme280-digitale-barometer-druk-en-vochtigheid-sensor-module)
[LCD Display](https://www.tinytronics.nl/nl/displays/lcd/lcd-display-16*2-karakters-met-zwarte-tekst-en-groen-gele-backlight-met-i2c-backpack)

## How to connect BME280 and LCD display to ESP32
COMPONENT | COLOUR | ESP
------------ | ---------- | -------------
BME280 VCC | RED | 3.3V
BME280 GND | BLACK | GND
BME280 SCL | GREEN | GPIO05
BME280 SDA | YELLOW | GPIO04
LCD VCC | ORANGE | 5V
LCD GNB | BLUE | GND
LCD SCL | YELLOW | GPIO05
LCD SDA | GREY | GPIO04

## I2C addresses

COMPONENT | ADDRESS
----------| --------
LCD DISPLAY | 0x27
BME280 | 0x77

# Pinlayout LoRa SX1278 Module ra-02

![image](https://images.tcdn.com.br/img/img_prod/557243/sx1278_lora_433mhz_ra_02_breakout_board_10km_959_1_20191128221303.png)

## Wiring LoRa SX1278 module with ESP32
SX1278 | COLOR | ESP32
-------- |-| ----------
3.3V | RED | 3.3V
GND | BLACK | GND
SCK | ORANGE | GPIO2
MOSI | WHITE | GPIO3
DIO0 | BROWN | GPIO6
NSS | PINK | GPIO7
MISO | GREY | GPIO10
RST | YELLOW | GPIO11

[Sample code](https://how2electronics.com/esp32-lora-sx1278-76-transmitter-receiver/)
