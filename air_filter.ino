#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PM25_PIN 2      // DSM501A PM2.5 output pin
#define SAMPLE_TIME 30000  // Sampling time in milliseconds (30s)

// Initialize LCD (I2C Address: 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    pinMode(PM25_PIN, INPUT);
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Air Quality:");
    Serial.begin(9600);
}

void loop() {
    unsigned long duration;
    unsigned long start_time = millis();
    unsigned long low_pulse_time = 0;

    // Measure low pulse duration for PM2.5
    while (millis() - start_time < SAMPLE_TIME) {
        duration = pulseIn(PM25_PIN, LOW);
        low_pulse_time += duration;
    }

    // Calculate PM2.5 concentration (approximation)
    float ratio = (low_pulse_time / (float)SAMPLE_TIME) * 100.0;
    float concentration = 0.5 * ratio;  // Approximate PM2.5 concentration in µg/m³

    // Convert PM2.5 concentration to AQI using the given formula
    int AQI = calculateAQI(concentration);

    // Display values on LCD
    lcd.setCursor(0, 1);
    lcd.print("PM2.5: ");
    lcd.print(concentration);
    lcd.print(" ug/m3");

    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AQI: ");
    lcd.print(AQI);
    lcd.setCursor(0, 1);
    lcd.print(getAQIDescription(AQI));

    Serial.print("PM2.5: ");
    Serial.print(concentration);
    Serial.print(" µg/m³, AQI: ");
    Serial.println(AQI);

    delay(5000);  // Update every 5 seconds
}

// Function to calculate AQI using the given formula
int calculateAQI(float C) {
    struct AQIRange {
        float Clow, Chigh;  // PM2.5 concentration range
        int Ilow, Ihigh;    // AQI range
    };

    AQIRange ranges[] = {
        {0.0, 12.0, 0, 50},
        {12.1, 35.4, 51, 100},
        {35.5, 55.4, 101, 150},
        {55.5, 150.4, 151, 200},
        {150.5, 250.4, 201, 300},
        {250.5, 350.4, 301, 400},
        {350.5, 500.4, 401, 500}
    };

    for (int i = 0; i < 7; i++) {
        if (C >= ranges[i].Clow && C <= ranges[i].Chigh) {
            return ((ranges[i].Ihigh - ranges[i].Ilow) / (ranges[i].Chigh - ranges[i].Clow)) * 
                   (C - ranges[i].Clow) + ranges[i].Ilow;
        }
    }
    return 500;  // Default to hazardous if out of range
}

// Function to get AQI category description
String getAQIDescription(int AQI) {
    if (AQI <= 50) return "Good";
    else if (AQI <= 100) return "Moderate";
    else if (AQI <= 150) return "Unhealthy for SG";
    else if (AQI <= 200) return "Unhealthy";
    else if (AQI <= 300) return "Very Unhealthy";
    else return "Hazardous";
}
