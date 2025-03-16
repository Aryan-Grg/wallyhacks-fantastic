#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_AHTX0.h>  // AHT10 sensor library
#include <Servo.h>           // Servo library

// Pin assignments for the TFT display
#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9

// Pin assignment for the servo
#define SERVO_PIN  3

// Create instances of the objects
Adafruit_AHTX0 aht;  // AHT10 sensor
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Servo servo;         // Create a servo object

// State tracking variables
bool servoAt90 = false;
bool crossedAbove80 = false;
bool crossedBelow80 = true;

void setup() {
  Serial.begin(115200);
  
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
  delay(100);
  digitalWrite(TFT_RST, LOW);
  delay(100);
  digitalWrite(TFT_RST, HIGH);
  delay(500);

  tft.initR(INITR_BLACKTAB);
  delay(500);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  if (!aht.begin()) {
    Serial.println("Failed to initialize AHT10 sensor!");
    while (1);
  }

  servo.attach(SERVO_PIN);
  servo.write(0);
  delay(500);
  servo.detach();
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  
  if (isnan(humidity.relative_humidity) || isnan(temp.temperature)) {
    Serial.println("Failed to read from AHT10 sensor!");
    return;
  }

  if (humidity.relative_humidity < 80.0) {
    tft.fillScreen(ST77XX_GREEN);
    if (!crossedBelow80) {
      servo.attach(SERVO_PIN);
      servo.write(0);
      delay(500);
      servo.detach();
      Serial.println("Humidity below 80%. Servo reset to 0 degrees.");
      crossedBelow80 = true;
      crossedAbove80 = false;
    }
  } else {
    tft.fillScreen(ST77XX_RED);
    if (!crossedAbove80) {
      servo.attach(SERVO_PIN);
      servo.write(100);
      delay(500);
      servo.detach();
      Serial.println("Humidity above 80%. Servo turned to 90 degrees.");
      crossedAbove80 = true;
      crossedBelow80 = false;
    }
  }

  tft.setTextSize(3);
  String humidityStr = String(humidity.relative_humidity, 1) + "%";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(humidityStr, 0, 0, &x1, &y1, &w, &h);
  int x = (tft.width() - w) / 2;
  int y = (tft.height() - h) / 2;

  tft.setCursor(x, y);
  tft.print(humidityStr);

  delay(2000);
}
