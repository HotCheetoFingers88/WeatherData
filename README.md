# WeatherData
Weather logger to monitor and log environmental data, including temperature, humidity, and atmospheric pressure

Displays the date, time, and current sensor readings on the LCD while logging the data to a CSV file on the SD card every 2 seconds:

Hardware Components:
* 20x4 LCD Screen: Utilizes the Adafruit I2C/SPI LCD backpack for easy display of readings
* DHT22 Sensor: Measures temperature and humidity
* Adafruit MPL115A2: Breakout board for measuring atmospheric pressure
* Adafruit Data Logger Shield: Enables data logging to an SD card

Circuit Connections:
* LCD (I2C Connection):
* 5V: Connect to Arduino 5V pin
* GND: Connect to Arduino GND pin
* CLK: Connect to Analog #5
* DAT: Connect to Analog #4

DHT22 Sensor:
* VCC: Connect to Arduino 5V pin
* GND: Connect to Arduino GND pin
* DAT: Connect to Digital #2
* 10K Resistor: Place between VCC and DAT as a pull-up resistor

MPL115A2 (I2C Connection):
* VCC: Connect to Arduino 5V pin
* GND: Connect to Arduino GND pin
* SCL: Connect to Analog #5
* SDA: Connect to Analog #4

Features:
* Real-time display of temperature, humidity, and pressure on the LCD
* Data logged every 2 seconds to a CSV file for easy analysis
* Timestamps for each data entry using a real-time clock
