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


void displayInformation() {
  // You can pass globals or retrieve from elsewhere in main
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Placeholder: actual drawing logic to be filled in main
  display.setCursor(0, 0);
  display.println("Weather info here");
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
