#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

// WiFi credentials
const char* ssid = "BSNL FIBER";
const char* password = "ihgi@ihgi";

// Server URL
const char* serverURL = "http://192.168.53.217:5000";

// Create RC522 instance
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// Create MLX90614 instance
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

String P_id = "";
float bodyTempF = 0;  // To store body temperature in Fahrenheit

void setup() {
  Serial.begin(115200);

  // Initialize SPI for RFID
  SPI.begin();
  rfid.PCD_Init();

  // Initialize I2C for MLX90614 sensor
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX90614 sensor.");
    while (1);  // Halt if sensor fails
  }

  // Initialize WiFi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Wait for RFID scan
  Serial.println("Please scan the patient's RFID tag...");
  while (P_id == "") {
    readRFIDTag();
  }

  // Get body temperature
  bodyTempF = getBodyTemperature();

  // Test server connectivity and send data
  testServerConnectivity();
  sendPatientData(P_id, bodyTempF);
}

void readRFIDTag() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  P_id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    P_id += String(rfid.uid.uidByte[i], HEX);
  }
  P_id.toUpperCase();
  Serial.println("Patient ID (RFID Tag) scanned: " + P_id);
  rfid.PICC_HaltA();
}

float getBodyTemperature() {
  // Read object temperature from MLX90614 sensor
  float objectTemp = mlx.readObjectTempC();
  // Convert to Fahrenheit
  return ((objectTemp * 9.0) / 5.0) + 32.0 + 5.0;
}

void testServerConnectivity() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    Serial.println("Pinging server...");

    if (http.begin(serverURL)) {
      Serial.println("Server reachable");
      http.end();
    } else {
      Serial.println("Server not reachable");
    }
  } else {
    Serial.println("WiFi not connected");
  }
}

void sendPatientData(String patientID, float temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(String(serverURL) + "/update_patient");
    http.addHeader("Content-Type", "application/json");

    // Send patient ID and temperature as JSON payload
    String jsonPayload = "{\"patientID\":\"" + patientID + "\", \"temperature\":\"" + String(temperature, 2) + "\"}";
    Serial.println("Sending JSON Payload: " + jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error in sending POST request. HTTP Response Code: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {
  // No need for loop in this case
}
