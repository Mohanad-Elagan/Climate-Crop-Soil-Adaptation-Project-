#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22
#include <NTPClient.h>
#include <WiFiUdp.h>

// The token generation process info.
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Keplar"
#define WIFI_PASSWORD "Eee112233445566!"

//Firebase project API Key
#define API_KEY "AIzaSyAxXxNbwREcwsyqgp9jWCjuQ9RZzoYAH8g"

#define USER_EMAIL "mohanad.elagan1@gmail.com"
#define USER_PASSWORD "112233445566"

// RTDB URLefine the RTDB URL
#define DATABASE_URL "https://group315-ccca6-default-rtdb.europe-west1.firebasedatabase.app/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int msensor = A0;
int msvalue = 0;
const int RELAY_PIN = 5; 
// Variable to save USER UID
String uid;

// Database main path
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String moisPath = "/pressure";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variable to save current epoch time
int timestamp;

float temperature;
float humidity;
float pressure;
// Timer variables
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}
DHT dht(DHTPIN, DHTTYPE);

void setup(){
  Serial.begin(115200);
  dht.begin();
 pinMode(msensor, INPUT);
 pinMode(RELAY_PIN, OUTPUT);
  initWiFi();
  timeClient.begin();

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  config.token_status_callback = tokenStatusCallback;

  // The maximum retry of token generation
  config.max_token_generation_retry = 5;

  // The library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(dht.readTemperature()));
    json.set(humPath.c_str(), String(dht.readHumidity()));
    json.set(moisPath.c_str(), String(analogRead(pressure)));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
msvalue = analogRead(msensor);

    if(msvalue > 420){
  digitalWrite(RELAY_PIN, HIGH);  // turn off pump 5 seconds
  delay(500);     
    }
  else{
  digitalWrite(RELAY_PIN, LOW); // turn on pump 5 seconds
  delay(500);
    }
}
