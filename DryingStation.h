#ifndef DRYING_STATION_H
#define DRYING_STATION_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ================================
// Configuration
// ================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

extern Adafruit_SSD1306 display;

const float RAIN_THRESHOLD_MM = 0.1;

// ================================
// Utility Functions
// ================================

float clamp(float x, float minVal, float maxVal);

// ================================
// OLED Functions
// ================================

void initOLED();
void printSplashScreen();
void resetOledForText();
void oledPrint(const String &text);
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
    int dryingScore);

// ================================
// Drying Score Functions
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
);

#endif // DRYING_STATION_H
