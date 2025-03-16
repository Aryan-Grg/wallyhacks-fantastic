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

// Flag to track whether the servo has moved
bool servoMoved = false;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Reset the TFT display
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);  // Ensure the reset pin is high
  delay(100);
  digitalWrite(TFT_RST, LOW);   // Pull reset low
  delay(100);
  digitalWrite(TFT_RST, HIGH);  // Bring reset high
  delay(500);  // Wait for the display to stabilize

  // Initialize the TFT display
  tft.initR(INITR_BLACKTAB);  // Use the correct option for your display
  delay(500);  // Add a short delay after initialization
  tft.setRotation(3); // Rotate display if needed
  tft.fillScreen(ST77XX_BLACK);  // Fill screen with black background
  tft.setTextColor(ST77XX_WHITE);
  
  // Initialize AHT10 sensor
  if (!aht.begin()) {
    Serial.println("Failed to initialize AHT10 sensor!");
    while (1);  // Halt if the sensor is not found
  }

  // Attach the servo to its pin
  servo.attach(SERVO_PIN);
  servo.write(0);  // Set servo to initial position (0 degrees)
}

void loop() {
  // Get humidity and temperature from the AHT10 sensor
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);  // Populate temp and humidity objects
  
  // If the readings are not valid, try again
  if (isnan(humidity.relative_humidity) || isnan(temp.temperature)) {
    Serial.println("Failed to read from AHT10 sensor!");
    return;
  }

  // Set background color based on humidity
  if (humidity.relative_humidity < 80.0) {
    tft.fillScreen(ST77XX_GREEN);  // Green background if humidity < 80%
  } else {
    tft.fillScreen(ST77XX_RED);    // Red background if humidity >= 80%
  }

  // Set text size for the big number
  tft.setTextSize(3);  // Large text size

  // Calculate the position to center the humidity value
  String humidityStr = String(humidity.relative_humidity, 1) + "%";  // Format humidity as a string
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(humidityStr, 0, 0, &x1, &y1, &w, &h);  // Get text dimensions
  int x = (tft.width() - w) / 2;  // Center horizontally
  int y = (tft.height() - h) / 2; // Center vertically

  // Display the humidity value
  tft.setCursor(x, y);
  tft.print(humidityStr);

  // Control the servo based on humidity
  if (humidity.relative_humidity < 50.0 && !servoMoved) {
    servo.write(90);  // Rotate servo to 90 degrees
    Serial.println("Humidity below 50%. Servo turned to 90 degrees.");
    servoMoved = true;  // Set flag to indicate servo has moved
    servo.detach();     // Detach the servo to stop sending control signals
  }

  // Wait for 2 seconds before updating the display
  delay(2000);
}