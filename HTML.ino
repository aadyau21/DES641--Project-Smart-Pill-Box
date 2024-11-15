#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Load Cell setup
#define LOADCELL_DOUT_PIN D4
#define LOADCELL_SCK_PIN D3
HX711 scale;

// RTC setup
RTC_DS3231 rtc;

// PIR sensor setup
#define PIR_PIN D2

// Wi-Fi credentials
const char* ssid = "iQOO 7";
const char* password = "abcdefgh";

// Web server setup
AsyncWebServer server(80);

// Variables
float previousWeight = 0;
bool pillTaken = false;
String pillStatus = "Pill Not Taken";

// User input variables
String pillName = "";
String pillTime = "";  // Morning, Afternoon, or Night
String compartmentNumber = "";

// Group Information
const String groupNumber = "Group MATRIX";
const String names = "Aadya Umrao, Manuja Pandey, Vanshika, Vaibhav Agarwal, Pulkit Sharma";
const String projectName = "Smart Pill Box";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.print("ESP8266 IP Address: ");
  Serial.print(WiFi.localIP());

  // Start web server
  server.on("/", HTTP_GET, [&groupNumber, &names, &projectName](AsyncWebServerRequest *request){
    // Construct the HTML response
    String htmlResponse = "<!DOCTYPE html><html><body>";
    htmlResponse += "<h1>Pill Box Status</h1>";
    htmlResponse += "<p>Status: " + pillStatus + "</p>";
    htmlResponse += "<h2>Group Information</h2>";
    htmlResponse += "<p><strong>Group Name:</strong> " + groupNumber + "</p>";
    htmlResponse += "<p><strong>Names:</strong> " + names + "</p>";
    htmlResponse += "<p><strong>Project Name:</strong> " + projectName + "</p>";
    // Send the response
    request->send(200, "text/html", htmlResponse);
  });
  server.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x57)) {
    Serial.println(F("OLED initialization failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Wi-Fi: Connected");
  display.display();
  delay(1000);

  // Initialize Load Cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  if (!scale.is_ready()) {
    Serial.println("Load cell initialization failed");
  } else {
    scale.tare();
    Serial.println("Load Cell: OK");
  }

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("RTC initialization failed");
  } else {
    Serial.println("RTC: OK");
  }

  // PIR sensor setup
  pinMode(PIR_PIN, INPUT);

  // Ask for user inputs via Serial Monitor
  Serial.println("Enter the Pill Name:");
  while (pillName == "") {
    if (Serial.available() > 0) {
      pillName = Serial.readStringUntil('\n');
      pillName.trim();
      Serial.print("Pill Name: ");
      Serial.println(pillName);
    }
  }

  Serial.println("Enter the Pill Taking Time (Morning, Afternoon, Night):");
  while (pillTime == "") {
    if (Serial.available() > 0) {
      pillTime = Serial.readStringUntil('\n');
      pillTime.trim();
      Serial.print("Pill Taking Time: ");
      Serial.println(pillTime);
    }
  }

  Serial.println("Enter the Compartment Number:");
  while (compartmentNumber == "") {
    if (Serial.available() > 0) {
      compartmentNumber = Serial.readStringUntil('\n');
      compartmentNumber.trim();
      Serial.print("Compartment Number: ");
      Serial.println(compartmentNumber);
    }
  }
}

void loop() {
  // Check Load Cell weight
  if (scale.is_ready()) {
    float currentWeight = scale.get_units(5);
    Serial.print("Weight: ");
    Serial.print(currentWeight);
    Serial.println(" kg");

    if (currentWeight < previousWeight) {
      pillTaken = true;
    }
    previousWeight = currentWeight;
  } else {
    Serial.println("Load Cell not ready");
  }

  // Get RTC time
  DateTime now = rtc.now();
  Serial.print("RTC Time: ");
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.println(now.second());

  String timeOfDay = "Night";
  if (now.hour() >= 6 && now.hour() < 12) {
    timeOfDay = "Morning";
  } else if (now.hour() >= 12 && now.hour() < 18) {
    timeOfDay = "Afternoon";
  }

  int pirValue = digitalRead(PIR_PIN);
  bool motionDetected = (pirValue == HIGH);
  Serial.println(motionDetected ? "PIR: Motion detected!" : "PIR: No motion");

  bool timeMatches = false;
  if ((pillTime == "Morning" && timeOfDay == "Morning") ||
      (pillTime == "Afternoon" && timeOfDay == "Afternoon") ||
      (pillTime == "Night" && timeOfDay == "Night")) {
    timeMatches = true;
  }

  if (pillTaken && motionDetected && timeMatches) {
    pillStatus = "Pill Taken";
    Serial.println(pillStatus);
  } else {
    pillStatus = "Pill Not Taken";
    Serial.println(pillStatus);
  }

  delay(1000);
}
