#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "confidentials.h"

#define REPORTING_PERIOD_MS     1000

// Firebase objects
FirebaseData firebaseData;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;
FirebaseJson json;

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;

void infiniteLoop() {
  Serial.println("Restart the ESP");
  for(;;);
}

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("♥ Beat!");
    // Grab the updated heart rate and SpO2 levels and send to Firebase
    if (Firebase.ready() && (millis() - tsLastReport > REPORTING_PERIOD_MS)) {
        tsLastReport = millis();
        float heartRate = pox.getHeartRate();
        int sp02 = pox.getSpO2();
        pox.shutdown();
        Serial.print("Heart rate: ");
        Serial.print(heartRate);
        Serial.print(" bpm / SpO2: ");
        Serial.print(sp02);
        Serial.println("%");
        json.set("/heartRate", heartRate);
        json.set("/sp02", sp02);
        Serial.printf("Send to firebase... %s\n", Firebase.RTDB.setJSON(&firebaseData, "user", &json) ? "ok" : firebaseData.errorReason().c_str());
        pox.resume();
    }
}

void setupWifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(300);
    }
    Serial.println("\nConnected with IP ");
    Serial.print(WiFi.localIP());
}

void setupFirebase() {
    firebaseConfig.api_key = API_KEY;
    firebaseConfig.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    firebaseData.setResponseSize(4096);
    firebaseConfig.token_status_callback = tokenStatusCallback; 

    if (Firebase.signUp(&firebaseConfig, &firebaseAuth, "", "")){
      Serial.println("\nFirebase initilization ok");
    } else {
      Serial.printf("%s\n", firebaseConfig.signer.signupError.message.c_str());
      infiniteLoop();
    }
    Firebase.begin(&firebaseConfig, &firebaseAuth);
}

void setupOximeter() {
    Serial.print("Initializing pulse oximeter..");
    // Initialize sensor
    if (!pox.begin()) {
        Serial.println("FAILED");
        infiniteLoop();
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
    setupWifi();
    setupFirebase();
    setupOximeter();
}

void loop() {
    // Read from the sensor
    pox.update();
}