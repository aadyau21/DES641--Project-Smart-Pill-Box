#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <RTClib.h>

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

// Variables
float previousWeight = 0;
bool pillTaken = false;

// User input variables
String pillName = "";
String pillTime = "";
String compartmentNumber = "";

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x57)) {
    Serial.println(F("OLED initialization failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("OLED: OK");
  display.display();
  delay(1000);

  // Initialize Load Cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  if (!scale.is_ready()) {
    Serial.println("Load cell initialization failed");
  } else {
    scale.tare(); // Reset the load cell (important for accurate readings)
    Serial.println("Load Cell: OK");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Load Cell: OK");
    display.display();
    delay(1000);
  }

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("RTC initialization failed");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("RTC Failed");
    display.display();
    delay(1000);
  } else {
    Serial.println("RTC: OK");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("RTC: OK");
    display.display();
    delay(1000);
  }

  // PIR sensor setup
  pinMode(PIR_PIN, INPUT);
  Serial.println("PIR Sensor: OK");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("PIR: OK");
  display.display();
  delay(1000);

  // Ask for user inputs via Serial Monitor
  Serial.println("Enter the Pill Name:");
  while (pillName == "") {
    if (Serial.available() > 0) {
      pillName = Serial.readStringUntil('\n');
      pillName.trim(); // Remove any unwanted whitespace
      Serial.print("Pill Name: ");
      Serial.println(pillName);
    }
  }

  Serial.println("Enter the Pill Taking Time (Morning, Afternoon, Night):");
  while (pillTime == "") {
    if (Serial.available() > 0) {
      pillTime = Serial.readStringUntil('\n');
      pillTime.trim(); // Remove any unwanted whitespace
      pillTime.toLowerCase(); // Convert to lowercase for consistency
      if (pillTime == "morning" || pillTime == "afternoon" || pillTime == "night") {
        Serial.print("Pill Taking Time: ");
        Serial.println(pillTime);
      } else {
        Serial.println("Invalid input. Please enter Morning, Afternoon, or Night.");
        pillTime = ""; // Reset if invalid
      }
    }
  }

  Serial.println("Enter the Compartment Number (e.g., 1, 2, etc.):");
  while (compartmentNumber == "") {
    if (Serial.available() > 0) {
      compartmentNumber = Serial.readStringUntil('\n');
      compartmentNumber.trim(); // Remove any unwanted whitespace
      Serial.print("Compartment Number: ");
      Serial.println(compartmentNumber);
    }
  }

  // Display input on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Pill Name: ");
  display.print(pillName);
  display.setCursor(0, 10);
  display.print("Pill Time: ");
  display.print(pillTime);
  display.setCursor(0, 20);
  display.print("Compartment: ");
  display.print(compartmentNumber);
  display.display();
}

void loop() {
  // Get RTC time
  DateTime now = rtc.now();
  String timeOfDay = "Night"; // Default to Night

  // Determine time of day: Morning (6:00-11:59), Afternoon (12:00-17:59), Night (18:00-5:59)
  if (now.hour() >= 6 && now.hour() < 12) {
    timeOfDay = "Morning";
  } else if (now.hour() >= 12 && now.hour() < 18) {
    timeOfDay = "Afternoon";
  }

  // Update OLED with RTC time and time of day
  display.setCursor(0, 0);
  display.print("RTC Time: ");
  display.print(now.hour() < 10 ? "0" : "");
  display.print(now.hour());
  display.print(':');
  display.print(now.minute() < 10 ? "0" : "");
  display.print(now.minute());
  display.print(':');
  display.print(now.second() < 10 ? "0" : "");
  display.print(now.second());
  display.print(" ");
  display.print(timeOfDay);
  display.display();

  // Check if the current time matches the pill taking time
  if (pillTime == timeOfDay) {
    int pirValue = digitalRead(PIR_PIN);
    display.setCursor(0, 32);  // Move to next line for PIR sensor status
    bool motionDetected = (pirValue == HIGH);
    
    if (motionDetected) {
      display.fillCircle(120, 50, 5, SSD1306_WHITE);  // Filled circle for Pill Taken
      Serial.println("Pill Taken");
      pillTaken = true;
    } else {
      display.drawCircle(120, 50, 5, SSD1306_WHITE);  // Unfilled circle for Pill Not Taken
      Serial.println("Pill Not Taken");
      pillTaken = false;
    }
  }

  // Display Pill Taken or Not status on OLED
  display.setCursor(0, 48);  // Move to the next line for Pill status
  if (pillTaken) {
    display.print("Pill Taken");
  } else {
    display.print("Pill Not Taken");
  }

  display.display();

  delay(1000); // Wait 1 second before re-testing
}
