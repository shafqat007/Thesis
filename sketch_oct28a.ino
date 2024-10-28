#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define RST_PIN 0       // GPIO0 for RFID reset pin
#define SS_PIN 2        // GPIO2 for RFID SS pin (SDA)
#define BUZZER_PIN 15   // GPIO15 for buzzer

// WiFi credentials
#define WIFI_SSID "Spotlight"
#define WIFI_PASSWORD "changedtoglitch"

// Firebase credentials
#define FIREBASE_HOST "https://rfid-296ab-default-rtdb.firebaseio.com/"  // Firebase Realtime Database URL
#define FIREBASE_EMAIL "shafqatnawazchy@gmail.com"        // Firebase email
#define FIREBASE_PASSWORD "123456"              // Firebase password

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
FirebaseData firebaseData;          // Firebase data instance
FirebaseConfig config;
FirebaseAuth auth;

String lastStatus = "gone";

void setup() {
  Serial.begin(115200);               // Initialize serial communications
  SPI.begin();                        // Initialize SPI bus
  mfrc522.PCD_Init();                 // Initialize RFID reader
  pinMode(BUZZER_PIN, OUTPUT);        // Set buzzer pin as output
  digitalWrite(BUZZER_PIN, LOW);      // Initially turn off buzzer

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Set Firebase configuration
  config.host = FIREBASE_HOST;
  config.api_key = "AIzaSyDApyR0KFpoSQDdimQImVq01sYH4uW9DaE";  // Add your Firebase project API key here
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  // Initialize Firebase with authentication
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Place your RFID card near the reader...");
}


void loop() {
  // Check if a new card is present
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;  // No card present
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;  // Could not read card
  }

  // Generate UID string
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }

  // Toggle status between "came" and "gone"
  String status = (lastStatus == "came") ? "gone" : "came";
  lastStatus = status;

  // Update Firebase Realtime Database
  String path = "/Tag_" + uid + "/status";
  if (Firebase.setString(firebaseData, path, status)) {
    Serial.println("Firebase database updated successfully.");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    Serial.println("Failed to update Firebase: " + firebaseData.errorReason());
  }
  delay(1000);
}
