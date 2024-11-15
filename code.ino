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

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x57)) {  // Use 0x57 or 0x68 based on the scanner result
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
}

void loop() {
  // Update OLED with Load Cell value
  if (scale.is_ready()) {
    float weight = scale.get_units(5); // Average over 5 readings
    Serial.print("Weight: ");
    Serial.print(weight);
    Serial.println(" kg");
    display.setCursor(0, 16);
    display.print("Weight: ");
    display.print(weight, 2);  // Print weight with 2 decimal places
    display.display();
  } else {
    Serial.println("Load Cell not ready");
  }

  // Test RTC
  DateTime now = rtc.now();
  Serial.print("RTC Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Update time on OLED (with 2-digit formatting)
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
  display.display();

  // Test PIR Sensor
  int pirValue = digitalRead(PIR_PIN);
  display.setCursor(0, 32);  // Move to next line for PIR sensor status
  if (pirValue == HIGH) {
    Serial.println("PIR: Motion detected!");
    display.print("PIR: Motion detected!");
  } else {
    Serial.println("PIR: No motion");
    display.print("PIR: No motion");
  }
  display.display();

  delay(1000); // Wait 1 second before re-testing
}