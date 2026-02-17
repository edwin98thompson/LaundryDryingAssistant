#include "DryingStation.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "qrcode_library.h"
#include "qrcode.h"


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================================
// Utility Functions
// ================================
float clamp(float x, float minVal, float maxVal) {
  if (x < minVal) return minVal;
  if (x > maxVal) return maxVal;
  return x;
}

// ================================
// OLED Functions
// ================================
void initOLED() {
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Allocation failed");
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Weather Station");
  display.println("----------------");
  display.println("Booting...");
  display.display();
}

void oledPrint(const String &text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.println(text);
  display.display();
}

void printSplashScreen()
{
  display.clearDisplay();

  // Draw shapes

  //id: 0 pixel 20 
  display.drawPixel(57, 28, SSD1306_WHITE);
  display.drawPixel(56, 28, SSD1306_WHITE);
  display.drawPixel(55, 28, SSD1306_WHITE);
  display.drawPixel(54, 28, SSD1306_WHITE);
  display.drawPixel(53, 28, SSD1306_WHITE);
  display.drawPixel(52, 28, SSD1306_WHITE);
  display.drawPixel(52, 29, SSD1306_WHITE);
  display.drawPixel(51, 29, SSD1306_WHITE);
  display.drawPixel(50, 29, SSD1306_WHITE);
  display.drawPixel(50, 30, SSD1306_WHITE);
  display.drawPixel(49, 31, SSD1306_WHITE);
  display.drawPixel(48, 32, SSD1306_WHITE);
  display.drawPixel(48, 33, SSD1306_WHITE);
  display.drawPixel(48, 34, SSD1306_WHITE);
  display.drawPixel(48, 35, SSD1306_WHITE);
  display.drawPixel(48, 36, SSD1306_WHITE);
  display.drawPixel(48, 37, SSD1306_WHITE);
  display.drawPixel(48, 38, SSD1306_WHITE);
  display.drawPixel(48, 39, SSD1306_WHITE);
  display.drawPixel(48, 40, SSD1306_WHITE);
  display.drawPixel(48, 41, SSD1306_WHITE);
  display.drawPixel(49, 41, SSD1306_WHITE);
  display.drawPixel(49, 42, SSD1306_WHITE);
  display.drawPixel(50, 42, SSD1306_WHITE);
  display.drawPixel(50, 43, SSD1306_WHITE);
  display.drawPixel(51, 44, SSD1306_WHITE);
  display.drawPixel(52, 45, SSD1306_WHITE);
  display.drawPixel(53, 45, SSD1306_WHITE);
  display.drawPixel(53, 46, SSD1306_WHITE);
  display.drawPixel(54, 46, SSD1306_WHITE);
  display.drawPixel(55, 47, SSD1306_WHITE);
  display.drawPixel(56, 47, SSD1306_WHITE);
  display.drawPixel(56, 48, SSD1306_WHITE);
  display.drawPixel(57, 48, SSD1306_WHITE);
  display.drawPixel(58, 48, SSD1306_WHITE);
  display.drawPixel(59, 48, SSD1306_WHITE);
  display.drawPixel(60, 49, SSD1306_WHITE);
  display.drawPixel(61, 49, SSD1306_WHITE);
  display.drawPixel(62, 49, SSD1306_WHITE);
  display.drawPixel(63, 49, SSD1306_WHITE);
  display.drawPixel(64, 49, SSD1306_WHITE);
  display.drawPixel(65, 49, SSD1306_WHITE);
  display.drawPixel(66, 50, SSD1306_WHITE);
  display.drawPixel(67, 50, SSD1306_WHITE);
  display.drawPixel(68, 50, SSD1306_WHITE);
  display.drawPixel(69, 51, SSD1306_WHITE);
  display.drawPixel(70, 51, SSD1306_WHITE);
  display.drawPixel(71, 51, SSD1306_WHITE);
  display.drawPixel(72, 51, SSD1306_WHITE);
  display.drawPixel(73, 51, SSD1306_WHITE);
  display.drawPixel(74, 52, SSD1306_WHITE);
  display.drawPixel(75, 52, SSD1306_WHITE);
  display.drawPixel(76, 52, SSD1306_WHITE);
  display.drawPixel(77, 52, SSD1306_WHITE);
  display.drawPixel(78, 52, SSD1306_WHITE);
  display.drawPixel(79, 53, SSD1306_WHITE);
  display.drawPixel(80, 53, SSD1306_WHITE);
  display.drawPixel(81, 53, SSD1306_WHITE);
  display.drawPixel(82, 53, SSD1306_WHITE);
  display.drawPixel(83, 53, SSD1306_WHITE);
  display.drawPixel(84, 53, SSD1306_WHITE);
  display.drawPixel(85, 53, SSD1306_WHITE);
  display.drawPixel(86, 53, SSD1306_WHITE);
  display.drawPixel(88, 53, SSD1306_WHITE);
  display.drawPixel(89, 53, SSD1306_WHITE);
  display.drawPixel(90, 53, SSD1306_WHITE);
  display.drawPixel(91, 53, SSD1306_WHITE);
  display.drawPixel(92, 52, SSD1306_WHITE);
  display.drawPixel(93, 52, SSD1306_WHITE);
  display.drawPixel(94, 52, SSD1306_WHITE);
  display.drawPixel(95, 51, SSD1306_WHITE);
  display.drawPixel(96, 51, SSD1306_WHITE);
  display.drawPixel(97, 51, SSD1306_WHITE);
  display.drawPixel(98, 51, SSD1306_WHITE);
  display.drawPixel(99, 51, SSD1306_WHITE);
  display.drawPixel(100, 50, SSD1306_WHITE);
  display.drawPixel(101, 50, SSD1306_WHITE);
  display.drawPixel(102, 50, SSD1306_WHITE);
  display.drawPixel(102, 49, SSD1306_WHITE);
  display.drawPixel(103, 49, SSD1306_WHITE);
  display.drawPixel(104, 49, SSD1306_WHITE);
  display.drawPixel(105, 49, SSD1306_WHITE);
  display.drawPixel(105, 48, SSD1306_WHITE);
  display.drawPixel(106, 48, SSD1306_WHITE);
  display.drawPixel(107, 47, SSD1306_WHITE);
  display.drawPixel(108, 47, SSD1306_WHITE);
  display.drawPixel(109, 47, SSD1306_WHITE);
  display.drawPixel(109, 46, SSD1306_WHITE);
  display.drawPixel(110, 46, SSD1306_WHITE);
  display.drawPixel(111, 45, SSD1306_WHITE);
  display.drawPixel(112, 45, SSD1306_WHITE);
  display.drawPixel(112, 44, SSD1306_WHITE);
  display.drawPixel(113, 43, SSD1306_WHITE);
  display.drawPixel(114, 42, SSD1306_WHITE);
  display.drawPixel(114, 41, SSD1306_WHITE);
  display.drawPixel(115, 40, SSD1306_WHITE);
  display.drawPixel(115, 39, SSD1306_WHITE);
  display.drawPixel(115, 38, SSD1306_WHITE);
  display.drawPixel(115, 37, SSD1306_WHITE);
  display.drawPixel(115, 36, SSD1306_WHITE);
  display.drawPixel(115, 35, SSD1306_WHITE);
  display.drawPixel(114, 34, SSD1306_WHITE);
  display.drawPixel(114, 33, SSD1306_WHITE);
  display.drawPixel(113, 33, SSD1306_WHITE);
  display.drawPixel(112, 33, SSD1306_WHITE);
  display.drawPixel(111, 32, SSD1306_WHITE);
  display.drawPixel(110, 32, SSD1306_WHITE);
  display.drawPixel(110, 31, SSD1306_WHITE);
  display.drawPixel(109, 31, SSD1306_WHITE);
  display.drawPixel(108, 31, SSD1306_WHITE);
  display.drawPixel(108, 30, SSD1306_WHITE);
  display.drawPixel(107, 30, SSD1306_WHITE);
  display.drawPixel(106, 30, SSD1306_WHITE);
  display.drawPixel(105, 30, SSD1306_WHITE);
  display.drawPixel(105, 29, SSD1306_WHITE);
  display.drawPixel(104, 29, SSD1306_WHITE);
  display.drawPixel(103, 29, SSD1306_WHITE);
  display.drawPixel(102, 29, SSD1306_WHITE);
  display.drawPixel(101, 29, SSD1306_WHITE);
  display.drawPixel(101, 28, SSD1306_WHITE);
  display.drawPixel(101, 27, SSD1306_WHITE);
  display.drawPixel(100, 27, SSD1306_WHITE);
  display.drawPixel(100, 26, SSD1306_WHITE);
  display.drawPixel(99, 25, SSD1306_WHITE);
  display.drawPixel(98, 25, SSD1306_WHITE);
  display.drawPixel(98, 24, SSD1306_WHITE);
  display.drawPixel(97, 24, SSD1306_WHITE);
  display.drawPixel(97, 23, SSD1306_WHITE);
  display.drawPixel(96, 23, SSD1306_WHITE);
  display.drawPixel(95, 23, SSD1306_WHITE);
  display.drawPixel(94, 23, SSD1306_WHITE);
  display.drawPixel(93, 23, SSD1306_WHITE);
  display.drawPixel(93, 22, SSD1306_WHITE);
  display.drawPixel(92, 22, SSD1306_WHITE);
  display.drawPixel(91, 22, SSD1306_WHITE);
  display.drawPixel(90, 22, SSD1306_WHITE);
  display.drawPixel(89, 22, SSD1306_WHITE);
  display.drawPixel(89, 23, SSD1306_WHITE);
  display.drawPixel(88, 23, SSD1306_WHITE);
  display.drawPixel(88, 22, SSD1306_WHITE);
  display.drawPixel(87, 21, SSD1306_WHITE);
  display.drawPixel(87, 20, SSD1306_WHITE);
  display.drawPixel(86, 20, SSD1306_WHITE);
  display.drawPixel(86, 19, SSD1306_WHITE);
  display.drawPixel(85, 19, SSD1306_WHITE);
  display.drawPixel(85, 18, SSD1306_WHITE);
  display.drawPixel(84, 18, SSD1306_WHITE);
  display.drawPixel(84, 17, SSD1306_WHITE);
  display.drawPixel(83, 17, SSD1306_WHITE);
  display.drawPixel(82, 17, SSD1306_WHITE);
  display.drawPixel(81, 17, SSD1306_WHITE);
  display.drawPixel(80, 17, SSD1306_WHITE);
  display.drawPixel(80, 16, SSD1306_WHITE);
  display.drawPixel(79, 16, SSD1306_WHITE);
  display.drawPixel(78, 16, SSD1306_WHITE);
  display.drawPixel(77, 16, SSD1306_WHITE);
  display.drawPixel(77, 17, SSD1306_WHITE);
  display.drawPixel(76, 17, SSD1306_WHITE);
  display.drawPixel(76, 18, SSD1306_WHITE);
  display.drawPixel(75, 19, SSD1306_WHITE);
  display.drawPixel(74, 19, SSD1306_WHITE);
  display.drawPixel(74, 20, SSD1306_WHITE);
  display.drawPixel(73, 19, SSD1306_WHITE);
  display.drawPixel(72, 19, SSD1306_WHITE);
  display.drawPixel(72, 18, SSD1306_WHITE);
  display.drawPixel(71, 18, SSD1306_WHITE);
  display.drawPixel(70, 18, SSD1306_WHITE);
  display.drawPixel(70, 17, SSD1306_WHITE);
  display.drawPixel(69, 17, SSD1306_WHITE);
  display.drawPixel(68, 17, SSD1306_WHITE);
  display.drawPixel(67, 17, SSD1306_WHITE);
  display.drawPixel(66, 17, SSD1306_WHITE);
  display.drawPixel(65, 17, SSD1306_WHITE);
  display.drawPixel(64, 17, SSD1306_WHITE);
  display.drawPixel(64, 18, SSD1306_WHITE);
  display.drawPixel(63, 18, SSD1306_WHITE);
  display.drawPixel(62, 18, SSD1306_WHITE);
  display.drawPixel(62, 19, SSD1306_WHITE);
  display.drawPixel(61, 19, SSD1306_WHITE);
  display.drawPixel(60, 20, SSD1306_WHITE);
  display.drawPixel(60, 21, SSD1306_WHITE);
  display.drawPixel(59, 21, SSD1306_WHITE);
  display.drawPixel(59, 22, SSD1306_WHITE);
  display.drawPixel(59, 23, SSD1306_WHITE);
  display.drawPixel(59, 24, SSD1306_WHITE);
  display.drawPixel(59, 25, SSD1306_WHITE);
  display.drawPixel(59, 26, SSD1306_WHITE);
  display.drawPixel(59, 27, SSD1306_WHITE);
  display.drawPixel(59, 28, SSD1306_WHITE);
  display.drawPixel(59, 29, SSD1306_WHITE);
  display.drawPixel(59, 30, SSD1306_WHITE);
  display.drawPixel(58, 30, SSD1306_WHITE);
  display.drawPixel(58, 29, SSD1306_WHITE);
  display.drawPixel(57, 29, SSD1306_WHITE);
  display.drawPixel(56, 29, SSD1306_WHITE);
  display.drawPixel(54, 27, SSD1306_WHITE);
  display.drawPixel(53, 27, SSD1306_WHITE);
  display.drawPixel(52, 27, SSD1306_WHITE);
  //id: 1 pixel 21 
  display.drawPixel(71, 31, SSD1306_WHITE);
  display.drawPixel(72, 31, SSD1306_WHITE);
  display.drawPixel(73, 31, SSD1306_WHITE);
  display.drawPixel(74, 31, SSD1306_WHITE);
  display.drawPixel(71, 32, SSD1306_WHITE);
  display.drawPixel(70, 32, SSD1306_WHITE);
  display.drawPixel(72, 32, SSD1306_WHITE);
  display.drawPixel(73, 32, SSD1306_WHITE);
  display.drawPixel(74, 32, SSD1306_WHITE);
  //id: 2 pixel 22 
  display.drawPixel(89, 31, SSD1306_WHITE);
  display.drawPixel(88, 31, SSD1306_WHITE);
  display.drawPixel(90, 31, SSD1306_WHITE);
  display.drawPixel(92, 31, SSD1306_WHITE);
  display.drawPixel(93, 30, SSD1306_WHITE);
  display.drawPixel(91, 30, SSD1306_WHITE);
  display.drawPixel(90, 30, SSD1306_WHITE);
  display.drawPixel(91, 31, SSD1306_WHITE);
  display.drawPixel(90, 32, SSD1306_WHITE);
  display.drawPixel(91, 32, SSD1306_WHITE);
  //id: 3 pixel 23 
  display.drawPixel(73, 39, SSD1306_WHITE);
  display.drawPixel(73, 40, SSD1306_WHITE);
  display.drawPixel(74, 40, SSD1306_WHITE);
  display.drawPixel(74, 41, SSD1306_WHITE);
  display.drawPixel(75, 41, SSD1306_WHITE);
  display.drawPixel(75, 42, SSD1306_WHITE);
  display.drawPixel(76, 42, SSD1306_WHITE);
  display.drawPixel(76, 43, SSD1306_WHITE);
  display.drawPixel(77, 43, SSD1306_WHITE);
  display.drawPixel(78, 43, SSD1306_WHITE);
  display.drawPixel(79, 43, SSD1306_WHITE);
  display.drawPixel(80, 43, SSD1306_WHITE);
  display.drawPixel(81, 44, SSD1306_WHITE);
  display.drawPixel(82, 44, SSD1306_WHITE);
  display.drawPixel(83, 44, SSD1306_WHITE);
  display.drawPixel(84, 44, SSD1306_WHITE);
  display.drawPixel(85, 44, SSD1306_WHITE);
  display.drawPixel(86, 44, SSD1306_WHITE);
  display.drawPixel(87, 44, SSD1306_WHITE);
  display.drawPixel(88, 44, SSD1306_WHITE);
  display.drawPixel(88, 43, SSD1306_WHITE);
  display.drawPixel(89, 43, SSD1306_WHITE);
  display.drawPixel(90, 42, SSD1306_WHITE);
  display.drawPixel(90, 41, SSD1306_WHITE);
  display.drawPixel(91, 41, SSD1306_WHITE);
  display.drawPixel(91, 40, SSD1306_WHITE);
  display.drawPixel(92, 39, SSD1306_WHITE);
  display.drawPixel(92, 38, SSD1306_WHITE);
  display.drawPixel(91, 39, SSD1306_WHITE);
  display.drawPixel(90, 40, SSD1306_WHITE);
  display.drawPixel(89, 41, SSD1306_WHITE);
  display.drawPixel(89, 42, SSD1306_WHITE);
  display.drawPixel(87, 43, SSD1306_WHITE);
  //id: 4 pixel 24 
  display.drawPixel(84, 53, SSD1306_WHITE);
  display.drawPixel(85, 53, SSD1306_WHITE);
  display.drawPixel(86, 53, SSD1306_WHITE);
  display.drawPixel(87, 53, SSD1306_WHITE);
  display.drawPixel(88, 53, SSD1306_WHITE);
  display.drawPixel(89, 53, SSD1306_WHITE);
  //id: 5 pixel 25 
  display.drawPixel(72, 55, SSD1306_WHITE);
  display.drawPixel(71, 55, SSD1306_WHITE);
  display.drawPixel(71, 56, SSD1306_WHITE);
  display.drawPixel(71, 57, SSD1306_WHITE);
  display.drawPixel(71, 58, SSD1306_WHITE);
  display.drawPixel(71, 59, SSD1306_WHITE);
  display.drawPixel(71, 60, SSD1306_WHITE);
  display.drawPixel(71, 61, SSD1306_WHITE);
  display.drawPixel(72, 61, SSD1306_WHITE);
  display.drawPixel(73, 61, SSD1306_WHITE);
  display.drawPixel(74, 61, SSD1306_WHITE);
  display.drawPixel(74, 60, SSD1306_WHITE);
  display.drawPixel(74, 59, SSD1306_WHITE);
  display.drawPixel(74, 58, SSD1306_WHITE);
  display.drawPixel(74, 57, SSD1306_WHITE);
  display.drawPixel(73, 57, SSD1306_WHITE);
  display.drawPixel(73, 56, SSD1306_WHITE);
  display.drawPixel(72, 56, SSD1306_WHITE);
  //id: 6 pixel 26 
  display.drawPixel(91, 56, SSD1306_WHITE);
  display.drawPixel(91, 57, SSD1306_WHITE);
  display.drawPixel(91, 58, SSD1306_WHITE);
  display.drawPixel(91, 59, SSD1306_WHITE);
  display.drawPixel(92, 59, SSD1306_WHITE);
  display.drawPixel(92, 60, SSD1306_WHITE);
  display.drawPixel(92, 61, SSD1306_WHITE);
  display.drawPixel(93, 61, SSD1306_WHITE);
  display.drawPixel(94, 61, SSD1306_WHITE);
  display.drawPixel(95, 61, SSD1306_WHITE);
  display.drawPixel(95, 60, SSD1306_WHITE);
  display.drawPixel(95, 59, SSD1306_WHITE);
  display.drawPixel(94, 59, SSD1306_WHITE);
  display.drawPixel(94, 58, SSD1306_WHITE);
  display.drawPixel(93, 58, SSD1306_WHITE);
  display.drawPixel(93, 57, SSD1306_WHITE);
  display.drawPixel(92, 57, SSD1306_WHITE);
  display.drawPixel(92, 56, SSD1306_WHITE);
  display.drawPixel(91, 55, SSD1306_WHITE);
  display.drawPixel(90, 55, SSD1306_WHITE);
  //id: 7 pixel 27 
  display.drawPixel(108, 53, SSD1306_WHITE);
  display.drawPixel(108, 54, SSD1306_WHITE);
  display.drawPixel(108, 55, SSD1306_WHITE);
  display.drawPixel(108, 56, SSD1306_WHITE);
  display.drawPixel(108, 57, SSD1306_WHITE);
  display.drawPixel(108, 58, SSD1306_WHITE);
  display.drawPixel(109, 58, SSD1306_WHITE);
  display.drawPixel(110, 58, SSD1306_WHITE);
  display.drawPixel(110, 59, SSD1306_WHITE);
  display.drawPixel(111, 59, SSD1306_WHITE);
  display.drawPixel(112, 59, SSD1306_WHITE);
  display.drawPixel(112, 58, SSD1306_WHITE);
  display.drawPixel(112, 57, SSD1306_WHITE);
  display.drawPixel(112, 56, SSD1306_WHITE);
  display.drawPixel(111, 56, SSD1306_WHITE);
  display.drawPixel(111, 55, SSD1306_WHITE);
  display.drawPixel(111, 54, SSD1306_WHITE);
  display.drawPixel(110, 54, SSD1306_WHITE);
  display.drawPixel(110, 53, SSD1306_WHITE);
  display.drawPixel(109, 53, SSD1306_WHITE);
  display.drawPixel(109, 52, SSD1306_WHITE);
  //id: 8 text 29 
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 1);
  display.print(F("Hi, its laundry buddy"));

  display.display();

  delay(5000);
}

void drawProvisioningQR(const String &serviceName, const String &pop) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];  // Version 3 fits OLED nicely

  String payload =
    "{\"ver\":\"v1\",\"name\":\"" + String(serviceName) +
    "\",\"pop\":\"" + String(pop) +
    "\",\"transport\":\"ble\"}";

  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, payload.c_str());

  display.clearDisplay();

  const int scale = 2;            // QR pixel scaling
  const int qrSize = qrcode.size * scale;
  const int xOffset = (128 - qrSize) / 2;
  const int yOffset = (64 - qrSize) / 2;

  for (int y = 0; y < qrcode.size; y++) {
    for (int x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(
          xOffset + x * scale,
          yOffset + y * scale,
          scale,
          scale,
          SSD1306_WHITE
        );
      }
    }
  }

  display.display();
}

void resetOledForText()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
}


void displayInformation(
    const String &city, 
    struct tm *timeinfo,
    double currentTempC,
    double currentHumidity,
    double currentWindMS,
    double currentPrecipMM,
    double currentRadiation,
    double currentEt0,
    int hoursTillRain,
    int dryingScore) 
{
  int year    = timeinfo->tm_year + 1900; // Offset by 1900
  int month   = timeinfo->tm_mon + 1;     // 0-indexed (Jan=0)
  int day     = timeinfo->tm_mday;
  int hour    = timeinfo->tm_hour;
  int minute  = timeinfo->tm_min;
  int second  = timeinfo->tm_sec;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.printf("%02d/%02d/%04d %02d:%02d:%02d",
                day, month, year,
                hour, minute, second);

  display.setCursor(0, 8);
  display.println(city);

  display.setCursor(0, 20);
  display.print("T: ");
  display.print(currentTempC, 1);

  // Get cursor position after printing the number
  int16_t x = display.getCursorX();
  int16_t y = display.getCursorY();

  // Draw small degree circle slightly above baseline
  display.drawCircle(x + 2, y - 2, 2, SSD1306_WHITE);

  // Move cursor slightly right so "C" doesn't overlap
  display.setCursor(x + 6, y);
  display.print("C  H: ");
  display.print(currentHumidity, 0);
  display.println("%");

  display.setCursor(0, 30);
  display.print("Wind: ");
  display.print(currentWindMS, 1);
  display.print("m/s  UV: ");
  display.print(currentRadiation, 0);

  display.setCursor(0, 40 );
  display.print("Drying Score: ");
  display.print(dryingScore, 1);

  if(hoursTillRain < 10)
  {
    display.setCursor(0, 50 );
    display.print("Rain in: ");
    display.print(hoursTillRain, 1);
    display.print(" hours");
  }

  display.display();
}

// ================================
// Drying Score Function
// ================================
void generateDryingScore(
    double currentTempC,
    double currentHumidity,
    double currentWindMS,
    double currentPrecipMM,
    double currentRadiation,
    double currentEt0,
    int hoursTillRain,
    int &dryingScore
) {
  // Hard stops
  if (currentPrecipMM > 0.1 || currentHumidity >= 95.0 || currentTempC < 3.0 || hoursTillRain <= 2) {
    dryingScore = 0;
    return;
  }

  // ET0 contribution
  float etScore = clamp(currentEt0 / 0.20, 0.0, 1.0) * 50.0;

  // Solar
  float solarScore = clamp(currentRadiation / 500.0, 0.0, 1.0) * 20.0;

  // Wind
  float windScore = clamp(currentWindMS / 5.0, 0.0, 1.0) * 15.0;

  // Humidity penalty
  float humidityPenalty = 0.0;
  if (currentHumidity > 50.0) {
    humidityPenalty = clamp((currentHumidity - 50.0) / 40.0, 0.0, 1.0) * 15.0;
  }

  // Temp modifier
  float tempMultiplier = 1.0;
  if (currentTempC < 10.0) {
    tempMultiplier = clamp(0.6 + (currentTempC - 3.0) / 7.0 * 0.4, 0.6, 1.0);
  }

  dryingScore = etScore + solarScore + windScore - humidityPenalty;
  dryingScore *= tempMultiplier;
  dryingScore = clamp(dryingScore, 0.0, 100.0);
}
