#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("♥ Beat!");
}

void setupOximeter() {
    Serial.print("Initializing pulse oximeter..");
    // Initialize sensor
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    // Configure sensor to use 7.6mA for LED drive
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    // Register a callback routine
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void setup() {
    Serial.begin(115200);
    setupOximeter();
}

void loop() {
    // Read from the sensor
    pox.update();

    // Grab the updated heart rate and SpO2 levels
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        float heartRate = pox.getHeartRate();
        int sp02 = pox.getSpO2();
        Serial.print("Heart rate: ");
        Serial.print(heartRate);
        Serial.print(" bpm / SpO2: ");
        Serial.print(sp02);
        Serial.println("%");
        tsLastReport = millis();
    }
}