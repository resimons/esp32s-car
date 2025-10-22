# Building a car controlled by an esp32s nodemcu

The ESP will read the data from the sensor and puts it on the LCD display.

## Pin layout ESP32


![image](https://github.com/resimons/esp32s-car/blob/main/images/ESP32-DEV-CH340-USBC-003.jpg)
![image](https://www.makerguides.com/wp-content/uploads/2020/08/DEBO_IR_38KHZ_01.jpg)
![image](https://www.tinytronics.nl/image/cache/data/product-2368/TM1637-RED4MINI-1200x1200w.jpg)
![image](https://www.tinytronics.nl/image/cache/data/product-3267/LCD1602-green-yellow-with-I2C-backpack-1500x1500w.jpg)

## Where to buy

[ESP-32S](https://www.benselectronics.nl/nodemcu-esp32-wroom-wifi-en-bluetooth-us-111566560.html)
[TM1637](https://www.tinytronics.nl/nl/displays/segmenten/segmenten-display-module-4-karakters-klok-rood-tm1637-mini)
[LCD-display](https://www.tinytronics.nl/nl/displays/lcd/lcd-display-16*2-karakters-met-zwarte-tekst-en-groen-gele-backlight-met-i2c-backpack)
[IR Receiver](https://www.tinytronics.nl/nl/communicatie-en-signalen/draadloos/infrarood/ir-infrarood-ontvanger-module-38khz-940nm)

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

## Wiring IR receiver
IR Receiver | COLOR | ESP
----- | - | ------
VCC | RED | 5V
GND | BLUE | GND
DATA | YELLOW | GPIO15

[Info](https://www.makerguides.com/ir-receiver-remote-arduino-tutorial/)
