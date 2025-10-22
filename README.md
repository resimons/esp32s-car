# Building a car controlled by an esp32s nodemcu

The ESP will read the data from the sensor and puts it on the LCD display.

## Pin layout ESP32

![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/esp32-esp-32s-nodemcu-pinout.jpg)
![image](https://github.com/resimons/esp32s-nodecmu-lora-bme280/blob/main/images/bme280.jpeg)
![image](https://www.tinytronics.nl/image/cache/data/product-2368/TM1637-RED4MINI-1200x1200w.jpg)
![image](https://www.tinytronics.nl/image/cache/data/product-3267/LCD1602-green-yellow-with-I2C-backpack-1500x1500w.jpg)

## Wiring

## Where to buy

[ESP-32S](https://elektronicavoorjou.nl/product/esp32-development-board-wifi-bluetooth)
[TM1637](https://www.tinytronics.nl/nl/displays/segmenten/segmenten-display-module-4-karakters-klok-rood-tm1637-mini)
[LCD-display](https://www.tinytronics.nl/nl/displays/lcd/lcd-display-16*2-karakters-met-zwarte-tekst-en-groen-gele-backlight-met-i2c-backpack)

## Wiring TM1637
VMA425 | COLOR | ESP32
-------- |-| ----------
VCC | RED | 3.3V
GND | BLACK | GND
CLK | YELLOW | GPIO25
DI0 | ORANGE | GPIO26

[Info](https://cdn.velleman.eu/downloads/29/vma425_a4v01.pdf)

## Wiring LCD display
LCD | COLOR | ESP
----- | - | ------
VCC | ORANGE | 5V
GND | BLUE | GND
SDA | YELLOW | GPIO21
SCL | GREY | GPIO22
