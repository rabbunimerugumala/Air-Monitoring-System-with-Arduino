// Include necessary libraries
#include <Wire.h>                 // I2C communication library
#include <LiquidCrystal_I2C.h>    // Library for I2C LCD display
#include <Adafruit_Sensor.h>      // General sensor library
#include <DHT.h>                  // DHT sensor library

// Define LCD (16x2) with I2C address 0x27 (change if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define sensor pins
#define MQ135_SENSOR A0           // MQ135 gas sensor connected to A0
#define DHTPIN 2                  // DHT sensor connected to pin 2
#define DHTTYPE DHT11             // Define DHT sensor type (DHT11 or DHT22)
#define BUZZER_PIN 3              // Buzzer connected to pin 3

DHT dht(DHTPIN, DHTTYPE);         // Create a DHT object

int gasLevel = 0;                 // Variable to store gas sensor reading
int baseline = 0;                  // Stores clean air baseline value
String airQuality = "";            // Stores air quality status

// Function to calibrate MQ135 sensor for a more realistic baseline
void calibrateMQ135() {
    Serial.println("Calibrating MQ135... Please wait...");
    long sum = 0;
    
    // Take multiple readings for calibration
    for (int i = 0; i < 100; i++) {
        sum += analogRead(MQ135_SENSOR);
        delay(50);
    }
    
    baseline = sum / 100; // Calculate average baseline value
    Serial.print("Baseline Set at: ");
    Serial.println(baseline);
}

// Function to read and display temperature & humidity
void sendSensor() {
    float h = dht.readHumidity();       // Read humidity
    float t = dht.readTemperature();    // Read temperature in Celsius

    // Check if readings are valid
    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        lcd.setCursor(0, 1);
        lcd.print("DHT Read Error   ");
        return;
    }

    // Display temperature & humidity on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: "); lcd.print(t); lcd.print("C   "); // Extra spaces to clear old data
    
    lcd.setCursor(0, 1);
    lcd.print("Humid: "); lcd.print(h); lcd.print("%  ");
}

// Function to trigger buzzer alerts
void triggerBuzzer(int beeps) {
    for (int i = 0; i < beeps; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
    }
}

// Function to read and display air quality realistically
void air_sensor() {
    gasLevel = analogRead(MQ135_SENSOR);  // Read gas sensor value
    Serial.print("Raw Gas Value: ");
    Serial.println(gasLevel);  // Print the raw value to check sensor response

    // Adjust air quality levels dynamically based on baseline
    int difference = gasLevel - baseline; // Change from baseline
    
    if (difference < 10) {
        airQuality = "GOOD     ";
    } else if (difference < 30) {
        airQuality = "MODERATE ";
    } else if (difference < 80) {
        airQuality = "UNHEALTHY";
        triggerBuzzer(1);  // One beep
    } else if (difference < 150) {
        airQuality = "VERY BAD ";
        triggerBuzzer(2);  // Two beeps
    } else {
        airQuality = "TOXIC    ";
        triggerBuzzer(3);  // Three beeps
    }

    // Display air quality on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gas: "); lcd.print(gasLevel); lcd.print("   ");
    
    lcd.setCursor(0, 1);
    lcd.print("Air: "); lcd.print(airQuality);
}

// Setup function (runs once at startup)
void setup() {
    Serial.begin(9600);            // Start serial communication
    pinMode(MQ135_SENSOR, INPUT);  // Set MQ135 sensor as input
    pinMode(BUZZER_PIN, OUTPUT);   // Set buzzer pin as output
    dht.begin();                   // Initialize DHT sensor

    // Initialize I2C LCD
    lcd.init();
    lcd.backlight();  // Turn on LCD backlight

    // Display startup message
    lcd.setCursor(2, 0);
    lcd.print("Air Quality");
    lcd.setCursor(4, 1);
    lcd.print("Monitor");
    delay(2000);
    lcd.clear();

    // Calibrate MQ135 sensor for better accuracy
    calibrateMQ135();
}

// Loop function (runs continuously)
void loop() {
    air_sensor();       // Read air quality
    delay(5000);        // Allow time for sensor to settle
    
    sendSensor();       // Read temperature & humidity
    delay(5000);
}
