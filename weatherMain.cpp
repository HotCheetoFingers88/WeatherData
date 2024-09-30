#include <SD.h>
#include <Wire.h>
#include <LiquidTWI.h>
#include <RTClib.h>
#include <DHT.h>
#include <Adafruit_MPL115A2.h>

// Constants
#define LOG_INTERVAL 2000       // Time interval (in milliseconds) between data entries
#define SYNC_INTERVAL 20000     // Time interval (in milliseconds) between file syncs
#define ECHO_TO_SERIAL 0        // Enable/disable echoing data to the serial port (0 = off, 1 = on)
#define WAIT_TO_START 0         // Enable/disable waiting for serial input during setup

// Pin Definitions
#define DHTPIN 2                // Digital pin for DHT sensor
#define DHTTYPE DHT22           // Define type of DHT sensor (DHT22)
const int chipSelect = 10;      // SD card chip select pin

// Sensor and Display Objects
LiquidTWI lcd(0);               // Initialize LCD using LiquidTWI library
RTC_DS1307 rtc;                 // Real-time clock object
DHT dht(DHTPIN, DHTTYPE);       // DHT sensor object
Adafruit_MPL115A2 mpl115a2;     // MPL115A2 pressure sensor object

// Log File
File logfile;                   // File object for logging data

// Day and Month Names for Display
const char* daysOfTheWeek[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; // Array of day names
const char* monthsOfTheYear[13] = {"   ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; // Array of month names

// Sensor Readings
float humidity = 0.0;           // Variable to store humidity reading
float temperatureF = 0.0;       // Variable to store temperature reading (in Fahrenheit)
float pressureKPA = 0.0;        // Variable to store pressure reading (in kPa)

// Error Handling Function
void error(const char *str) {
    lcd.clear();                // Clear the LCD display
    lcd.print("Error: ");       // Display error message
    lcd.println(str);           // Print specific error message
    #if ECHO_TO_SERIAL
    Serial.println(str);        // Optionally print error message to serial port
    #endif
    while (1);                  // Halt execution to prevent further operation
}

// Setup Function
void setup() {
    Serial.begin(9600);         // Start serial communication at 9600 baud
    Serial.println("Starting Weather Logger..."); // Indicate start of the logger

    // LCD Initialization
    lcd.begin(20, 4);           // Initialize LCD with 20 columns and 4 rows
    lcd.print("   Weather Logger"); // Display initial message on LCD

    // SD Card Initialization
    pinMode(chipSelect, OUTPUT); // Set chip select pin as output
    if (!SD.begin(chipSelect)) {  // Initialize SD card
        error("SD Card failed");   // Handle SD card failure
    }
    Serial.println("SD Card initialized."); // Confirm SD card initialization
    lcd.println("SD Card OK");    // Indicate SD card is ready

    // Create Log File
    char filename[] = "LOGGER00.CSV"; // Define filename for logging
    createLogFile(filename);        // Create log file with available filename

    // Initialize Sensors
    dht.begin();                   // Start DHT sensor
    if (!mpl115a2.begin()) {      // Initialize MPL115A2 sensor
        error("MPL115A2 failed"); // Handle sensor failure
    }

    // RTC Initialization
    if (!rtc.begin() || !rtc.isrunning()) { // Initialize RTC and check if it's running
        if (!rtc.isrunning()) {              // If RTC is not running
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to compile time
        }
        error("RTC failed");                  // Handle RTC failure
    }

    // Log file header
    logfile.println("millis,stamp,datetime,humidity,temp,pressure"); // Log file header for CSV
    #if ECHO_TO_SERIAL
    Serial.println("millis,stamp,datetime,humidity,temp,pressure"); // Optionally print header to serial
    #endif
}

// Create Log File Function
void createLogFile(char* filename) {
    for (uint8_t i = 0; i < 100; i++) { // Loop to find available filename
        filename[6] = '0' + (i / 10);     // Set tens digit
        filename[7] = '0' + (i % 10);     // Set units digit
        if (!SD.exists(filename)) {        // Check if file exists
            logfile = SD.open(filename, FILE_WRITE); // Open file for writing
            return;                       // Exit function after file creation
        }
    }
    error("Couldn't create file");       // Handle file creation failure
}

// Loop Function
void loop() {
    static uint32_t syncTime = 0;      // Variable to track sync timing
    static uint32_t lastLogTime = 0;   // Variable to track last log timing

    // Check if it's time to log data
    if (millis() - lastLogTime >= LOG_INTERVAL) {
        lastLogTime = millis();         // Update last log time
        logData();                     // Log sensor data
    }
  
    // Sync to SD card if required
    if (millis() - syncTime >= SYNC_INTERVAL) {
        syncTime = millis();            // Update sync time
        logfile.flush();                // Ensure data is written to SD card
    }
}

// Log Data Function
void logData() {
    uint32_t m = millis();             // Get current time in milliseconds
    DateTime now = rtc.now();          // Get current date and time from RTC

    // Log Time Data
    logfile.print(m);                  // Log elapsed time
    logfile.print(", ");
    logfile.print(now.unixtime());     // Log Unix timestamp
    logfile.print(", \"");
    logfile.print(now.year());          // Log year
    logfile.print("/");
    logfile.print(now.month());         // Log month
    logfile.print("/");
    logfile.print(now.day());           // Log day
    logfile.print(" ");
    logfile.print(now.hour());          // Log hour
    logfile.print(":");
    logfile.print(now.minute());        // Log minute
    logfile.print(":");
    logfile.print(now.second());        // Log second
    logfile.print("\", ");

    // Read Sensors
    readSensors();                     // Read sensor data

    // Log Sensor Data
    logfile.print(humidity);           // Log humidity
    logfile.print(", ");
    logfile.print(temperatureF);       // Log temperature
    logfile.print(", ");
    logfile.print(pressureKPA);        // Log pressure
    logfile.println();                  // End line for new entry

    #if ECHO_TO_SERIAL
    printLogToSerial(m, now);         // Optionally print log to serial
    #endif

    // Update LCD Display
    updateLCD(now);                   // Update LCD with current data
}

// Read Sensors Function
void readSensors() {
    humidity = dht.readHumidity();     // Read humidity from DHT sensor
    temperatureF = dht.readTemperature(true); // Read temperature in Fahrenheit

    // Validate Sensor Readings
    if (isnan(humidity) || isnan(temperatureF)) { // Check for valid readings
        error("Failed to read from DHT sensor"); // Handle reading failure
        return; // Exit function if reading failed
    }

    pressureKPA = mpl115a2.getPressure(); // Read pressure from MPL115A2 sensor
}

// Print Log to Serial Function
void printLogToSerial(uint32_t m, const DateTime& now) {
    Serial.print(m);                   // Print elapsed time
    Serial.print(", ");
    Serial.print(now.unixtime());      // Print Unix timestamp
    Serial.print(", \"");
    Serial.print(now.year());           // Print year
    Serial.print("/");
    Serial.print(now.month());          // Print month
    Serial.print("/");
    Serial.print(now.day());            // Print day
    Serial.print(" ");
    Serial.print(now.hour());           // Print hour
    Serial.print(":");
    Serial.print(now.minute());         // Print minute
    Serial.print(":");
    Serial.print(now.second());         // Print second
    Serial.print("\", ");
    Serial.print(humidity);             // Print humidity
    Serial.print(", ");
    Serial.print(temperatureF);         // Print temperature
    Serial.print(", ");
    Serial.print(pressureKPA);          // Print pressure
    Serial.println();                   // New line for next entry
}

// Update LCD Function
void updateLCD(const DateTime& now) {
    lcd.setCursor(0, 1);               // Set cursor position on the second row
    lcd.printf("%s %s %02d  %02d:%02d:%02d",
        daysOfTheWeek[now.dayOfTheWeek()], // Display day of the week
        monthsOfTheYear[now.month()],      // Display month
        now.day(),                          // Display day of the month
        now.hour(),                         // Display hour
        now.minute(),                       // Display minute
        now.second());                      // Display second

    lcd.setCursor(0, 2);               // Move to the third row
    lcd.printf("H: %.1f%% Temp: %.1fF", humidity, temperatureF); // Display humidity and temperature
    
    lcd.setCursor(0, 3);               // Move to the fourth row
    lcd.printf("Press: %.1f kPa", pressureKPA); // Display pressure
}
