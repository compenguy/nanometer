# A battery-powered wifi temperature sensor with display

Parts:
* [Arduino nano rp2040 connect](https://docs.arduino.cc/hardware/nano-rp2040-connect/)
* [WeAct 2.9" e-paper display](https://github.com/WeActStudio/WeActStudio.EpaperModule)
* [Adafruit PowerBoost 500 USB LiPo Charger](https://www.adafruit.com/product/1944)
* [3.7V 500mAh LiPo Battery](https://www.adafruit.com/product/2011)

We make use of the built-in [LSM6DSOX IMU's temperature sensor](https://docs.arduino.cc/tutorials/nano-rp2040-connect/rp2040-01-technical-reference/#temperature)
for taking readings, the SPI bus and 3V3 line to power and update the e-paper
display, and the PowerBoost 500 to interface with and manage the battery.

Because we don't use the IMU itself, we can keep power utilization to an
absolute minimum, polling just the temperature sensor, updating the epaper
display if the temperature changed, and publishing the new value over wifi,
and sleeping in between each poll.

# Controller pin assignments

Controller connections to the PowerBoost 500 module:
* VIN <- battery power +
* GND <- battery power -

 Controller connections to the WeAct e-paper display:
* +3V3 -> display supply power + (VCC, pin 8)
* GND  -> display supply power - (GND, pin 7)
* D11/SPI0_TX  -> I/O bus data   (SDA, pin 6)
* D13/SPI0_SCK -> I/O bus clock  (SCL, pin 5)
* D10/SPI0_CSn -> I/O chip select (CS, pin 4)
* GPIOa -> data/command control  (D/C, pin 3)
* GPIOb -> reset (active low)    (RES, pin 2)
* GPIOc <- busy interrupt        (BSY, pin 1)
