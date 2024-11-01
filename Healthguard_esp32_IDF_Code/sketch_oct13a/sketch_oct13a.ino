#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "Mayank's A14";
const char* password = "12345678";

const char* serverURL = "http://192.168.53.217:5000";
String P_id = "";

// Define the RC522 module pins
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);  // Create instance of the RC522 class

void setup() {
  Serial.begin(115200);
  
  // Initialize the RC522 module
  SPI.begin();        // SPI bus initialization
  rfid.PCD_Init();    // Initialize the MFRC522
  
  // Initialize WiFi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Wait for RFID scan to get the Patient ID
  Serial.println("Please scan the patient's RFID tag...");
  while (P_id == "") {
    readRFIDTag(); // Wait until an RFID tag is scanned
  }
  
  testServerConnectivity();
  
  sendPatientID(P_id);
}

void readRFIDTag() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) return;
  // Verify if the card can be read
  if (!rfid.PICC_ReadCardSerial()) return;
  
  // Construct a patient ID from the UID
  P_id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    P_id += String(rfid.uid.uidByte[i], HEX); // Convert UID bytes to a hex string
  }
  P_id.toUpperCase();  // Convert to uppercase
  Serial.println("Patient ID (RFID Tag) scanned: " + P_id);
  
  // Halt PICC (stop communication with the card)
  rfid.PICC_HaltA();
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

void sendPatientID(String patientID) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(String(serverURL) + "/update_patient");
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"patientID\":\"" + patientID + "\"}";
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
  // No need for loop code in this case as everything is done in setup.
}
