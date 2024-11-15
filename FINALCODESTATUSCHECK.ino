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
String pillTime = "";  // Morning, Afternoon, or Night
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
      Serial.print("Pill Taking Time: ");
      Serial.println(pillTime);
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
  // Check Load Cell weight
  if (scale.is_ready()) {
    float currentWeight = scale.get_units(5); // Average over 5 readings
    Serial.print("Weight: ");
    Serial.print(currentWeight);
    Serial.println(" kg");

    // Check if the weight has decreased
    if (currentWeight < previousWeight) {
      pillTaken = true;
    }

    previousWeight = currentWeight;  // Update the previous weight for next loop
  } else {
    Serial.println("Load Cell not ready");
  }

  // Get RTC time
  DateTime now = rtc.now();
  Serial.print("RTC Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(' ');
  Serial.print(now.minute(), DEC);
  Serial.print(' ');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Determine time of day: Morning (6:00-11:59), Afternoon (12:00-17:59), Night (18:00-5:59)
  String timeOfDay = "Night";
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

  // Check PIR Sensor for motion
  int pirValue = digitalRead(PIR_PIN);
  display.setCursor(0, 32);  // Move to next line for PIR sensor status
  bool motionDetected = (pirValue == HIGH);
  if (motionDetected) {
    Serial.println("PIR: Motion detected!");
  } else {
    Serial.println("PIR: No motion");
  }

  // Check if pill taking time matches with current time of day
  bool timeMatches = false;
  if ((pillTime == "Morning" && timeOfDay == "Morning") ||
      (pillTime == "Afternoon" && timeOfDay == "Afternoon") ||
      (pillTime == "Night" && timeOfDay == "Night")) {
    timeMatches = true;
  }

  // Display Pill Taken or Not based on conditions
  display.setCursor(0, 48);  // Move to the next line for Pill status
  if (pillTaken && motionDetected && timeMatches) {
    display.fillCircle(120, 50, 5, SSD1306_WHITE);  // Filled circle
    Serial.println("Pill Taken");
  } else {
    display.drawCircle(120, 50, 5, SSD1306_WHITE);  // Unfilled circle
    Serial.println("Pill Not Taken");
  }

  display.display();

  delay(1000); // Wait 1 second before re-testing
}
