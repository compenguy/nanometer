# A battery-powered wifi temperature sensor with display

Parts:
* [Arduino nano rp2040 connect](https://docs.arduino.cc/hardware/nano-rp2040-connect/)
* [WeAct 2.9" e-paper display](https://github.com/WeActStudio/WeActStudio.EpaperModule)
* [Adafruit PowerBoost 500 USB LiPo Charger](https://www.adafruit.com/product/1944)
* [3.7V 500mAh LiPo Battery](https://www.adafruit.com/product/2011)
* [Adafruit AHT20 Temp Sensor](https://www.adafruit.com/product/4566)

During early development, it's handy to use the built-in LSM6DSOX IMU's
[built-in temperature sensor](https://docs.arduino.cc/tutorials/nano-rp2040-connect/rp2040-01-technical-reference/#temperature),
but because that's a reading taken from a heat-generating component, it's
always going to read quite high. Eventually we'll have to switch over to a
dedicated sensor (the AHT20) that we can isolate from heat sources, connected
via the I2C bus.

For display, we use the SPI bus and 3V3 line to power and update the e-paper
display.

When we're ready to go fully wireless, we add the PowerBoost 500 to interface
with and manage the battery.

# Controller pin assignments

Controller connections to the WeAct e-paper display, following the
[library-recommended pins for this uController](https://github.com/ZinggJM/GxEPD2/blob/master/ConnectingHardware.md#mapping-suggestion-for-arduino-nano-rp2040-connect-arduino-mbed-os-nano-boards):

* 5V   -> display supply power + (VCC, pin 8)
* GND  -> display supply power - (GND, pin 7)
* D11/SPI0_TX  -> I/O bus data   (SDA, pin 6)
* D13/SPI0_SCK -> I/O bus clock  (SCL, pin 5)
* D10/SPI0_CSn -> I/O chip select (CS, pin 4)
* D8/GPIO20 -> data/command control  (D/C, pin 3)
* D9/GPIO21 -> reset (active low)    (RES, pin 2)
* D7/GPIO19 <- busy interrupt        (BSY, pin 1)

Controller connections for the AHT20 sensor:

* +3V3        -> sensor supply power + (VIN 3V3)
* GND         -> sensor supply power - (GND)
* D18/A4/SDA <-> I/O bus data (SDA)
* D19/A5/SCL  -> I/O bus clock (SCL)

Controller connections to the PowerBoost 500 module:
* VIN <- battery power +
* GND <- battery power -

