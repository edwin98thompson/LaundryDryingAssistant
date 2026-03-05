#include <WiFi.h>
#include <WiFiProv.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "time.h"
#include "DryingStation.h"  // OLED, clamp, and drying score

// Set to false in a production setting
bool usingAtHome = true;

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

RTC_DATA_ATTR bool forceProvisioning = false;

// ---------- Provisioning Configuration ----------
const char *POP = "abcd1234";         // Proof-of-Possession PIN
const char *SERVICE_NAME = "PROV_1";  // BLE name (must start with PROV_)
const char *SERVICE_KEY = NULL;       // Not used for BLE
bool RESET_PROVISIONED = false;

uint8_t SERVICE_UUID[16] = {
  0xb4, 0xdf, 0x5a, 0x1c,
  0x3f, 0x6b, 0xf4, 0xbf,
  0xea, 0x4a, 0x82, 0x03,
  0x04, 0x90, 0x1a, 0x02
};

// -- Control Button ------------------
const int pushButton = 23;
const unsigned long LONG_PRESS_TIME = 5000;  // Duration in milliseconds

unsigned long pressStartTime = 0;
bool isPressing = false;
bool longPressHandled = false;

// ---------- State Tracking ----------
int httpGetRetries = 3;
bool wifiReady = false;
volatile bool wifiJustConnected = false;
static int wifiFailures = 0;

double latitude = 0.0;
double longitude = 0.0;
String city;

double currentTempC = NAN;
double currentHumidity = NAN;
double currentWindMS = NAN;
double currentPrecipMM = NAN;
double currentRadiation = NAN;
double currentEt0 = NAN;
int hoursTillRain = 0;
int dryingScore = 0;

// ---------- Wi-Fi Event Handler ----------
void onWiFiEvent(arduino_event_t *event) {
  int connectionFailureCount = 0;
  switch (event->event_id) {
    case ARDUINO_EVENT_PROV_START:
      Serial.println("[PROV] BLE provisioning started");
      oledPrint("BLE provisioning started...");
      break;

    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("[PROV] Provisioning successful");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("[WIFI] Connected. IP: ");
      Serial.println(IPAddress(event->event_info.got_ip.ip_info.ip.addr));
      wifiReady = true;
      wifiJustConnected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifiReady = false;
      oledPrint("WiFi Error\nRetrying...");
      break;

    default:
      break;
  }
}

// ---------- Provisioning Startup ----------
void startProvisioning() {
  Serial.println("[SYS] Starting BLE provisioning");

  delay(500);

  WiFiProv.beginProvision(
    NETWORK_PROV_SCHEME_BLE,
    NETWORK_PROV_SCHEME_HANDLER_FREE_BLE,
    NETWORK_PROV_SECURITY_1,
    POP,
    SERVICE_NAME,
    SERVICE_KEY,
    SERVICE_UUID,
    RESET_PROVISIONED);

  // QR code for app scanning
  Serial.println("[SYS] QR code for Serial Monitor (scan with phone):");
  WiFiProv.printQR(SERVICE_NAME, POP, "ble");
  Serial.println("[SYS] End of QR code");
}

void reprovisionDevice() {
  Serial.println("[SYS] Reprovisioning requested");

  oledPrint("Factory Reset\nReprovisioning");
  delay(1000);

  forceProvisioning = true;

  WiFi.setAutoReconnect(false);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);

  delay(1000);
  ESP.restart();
}

// ---------- Location & Time ----------
void getLocation() {
  int attempts = 0;
  int httpCode = -1;
  HTTPClient http;

  if (usingAtHome) {
    city = SECRET_CITY;
    latitude = SECRET_LATITUDE;
    longitude = SECRET_LONGITUDE;

    Serial.printf("City: %s, Lat: %.4f, Lon: %.4f\n", city.c_str(), latitude, longitude);
    delay(2000);
    return;
  }

  while (attempts < httpGetRetries) {
    http.begin("https://ipwho.is/");

    httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      break;
    }

    http.end();
    attempts++;
    delay(1000);
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("[GEO] HTTP request failed");
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<1024> doc;
  if (deserializeJson(doc, payload)) {
    Serial.println("[GEO] JSON parse failed");
    return;
  }

  city = doc["city"] | "";
  latitude = doc["latitude"] | 0.0;
  longitude = doc["longitude"] | 0.0;

  Serial.printf("City: %s, Lat: %.4f, Lon: %.4f\n", city.c_str(), latitude, longitude);
  // oledPrint(city + "\nLat:" + String(latitude,4) + " Lon:" + String(longitude,4));
  delay(2000);
}

// see if this is the best way to sync time and look into reusability
void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println();

  if (retries >= 20) {
    Serial.println("[TIME] Failed to sync time");
    display.println("Failed to sync time");
    display.display();
  } else {
    Serial.println("[TIME] Time synced");
    display.println("Time synced");
    display.display();
  }
  delay(1000);
}

// ---------- Weather Fetch ----------
void fetchWeather() {

  // ---- Ensure WiFi is connected ----
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WEATHER] Wi-Fi not connected");
    return;
  }

  // ---- Build Open-Meteo URL ----
  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=" + String(latitude, 6);
  url += "&longitude=" + String(longitude, 6);
  url += "&minutely_15=temperature_2m,relative_humidity_2m,wind_speed_10m,precipitation,shortwave_radiation";
  url += "&hourly=et0_fao_evapotranspiration";
  url += "&temperature_unit=celsius&timezone=auto";  // Return timestamps in local timezone
  url += "&forecast_days=1";

  int attempts = 0;
  int httpCode = -1;
  HTTPClient http;

  while (attempts < httpGetRetries) {
    http.begin(url);

    httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      break;
    }

    http.end();
    attempts++;
    delay(1000);
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[WEATHER] HTTP GET failed, code: %d\n", httpCode);
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, payload)) {
    Serial.println("[WEATHER] JSON parse failed");
    return;
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  int roundedMinute = ((currentMinute + 14) / 15) * 15;

  if (roundedMinute == 60) {
    roundedMinute = 0;
    currentHour++;
  }

  int currentQuarter = roundedMinute / 15;
  int currentIndex = currentHour * 4 + currentQuarter;

  // Access data
  JsonArray temps = doc["minutely_15"]["temperature_2m"].as<JsonArray>();
  JsonArray winds = doc["minutely_15"]["wind_speed_10m"].as<JsonArray>();
  JsonArray humidities = doc["minutely_15"]["relative_humidity_2m"].as<JsonArray>();
  JsonArray precip = doc["minutely_15"]["precipitation"].as<JsonArray>();
  JsonArray radiation = doc["minutely_15"]["shortwave_radiation"].as<JsonArray>();
  JsonArray et0_values = doc["hourly"]["et0_fao_evapotranspiration"].as<JsonArray>();

  if (currentIndex >= temps.size()) return;

  // Rain detection
  hoursTillRain = 12;
  for (size_t j = currentIndex; j < currentIndex + 48 && j < precip.size(); j++) {
    if (precip[j].as<float>() > RAIN_THRESHOLD_MM) {
      hoursTillRain = (j - currentIndex) / 4;
      Serial.println("Rain expected in " + String(hoursTillRain) + " hours");
      break;
    }
  }

  currentTempC = temps[currentIndex].as<float>();
  currentHumidity = humidities[currentIndex].as<float>();
  currentWindMS = winds[currentIndex].as<float>();
  currentPrecipMM = precip[currentIndex].as<float>();
  currentRadiation = radiation[currentIndex].as<float>();
  currentEt0 = et0_values[currentHour].as<float>();

  Serial.printf("[WEATHER] H%02d | T=%.2fC H=%.1f%% W=%.2fm/s P=%.2fmm R=%.1f ET0=%.3f\n",
                currentHour,
                currentTempC,
                currentHumidity,
                currentWindMS,
                currentPrecipMM,
                currentRadiation,
                currentEt0);
}

void checkButtonState() {
  int buttonState = digitalRead(pushButton);

  // Button is pressed (LOW due to INPUT_PULLUP)
  if (buttonState == LOW) {
    if (!isPressing) {
      isPressing = true;
      pressStartTime = millis();  // Start timer
      longPressHandled = false;
    }

    // Check if enough time has passed while still held
    if (!longPressHandled && (millis() - pressStartTime >= LONG_PRESS_TIME)) {
      Serial.println("Long press detected!");
      longPressHandled = true;  // Prevents repeated triggers
      reprovisionDevice();
    }
  }
  // Button is released
  else {
    if (isPressing) {
      isPressing = false;
      unsigned long duration = millis() - pressStartTime;
      Serial.print("Button released. Total duration: ");
      Serial.println(duration);
    }
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  pinMode(pushButton, INPUT_PULLUP);
  delay(300);

  initOLED();
  printSplashScreen();
  resetOledForText();
  display.println("Init WiFi...");
  display.display();

  if (!usingAtHome) {
    WiFi.onEvent(onWiFiEvent);

    // HARD stop WiFi before provisioning
    WiFi.mode(WIFI_OFF);
    WiFi.setAutoReconnect(false);
    delay(500);

    if (forceProvisioning) {
      Serial.println("[SYS] Forced provisioning mode");
      forceProvisioning = false;
      startProvisioning();
      return;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[SYS] Already provisioned, connecting to Wi-Fi");
      WiFi.begin();
    } else {
      startProvisioning();
    }
  } else {
    WiFi.begin(ssid, pass);
    Serial.println("\nConnecting with hard coded credentials");

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }

    Serial.println("\nConnected to WiFi network");
    display.println("Connected to WiFi network");
    display.display();
    wifiJustConnected = true;
    wifiReady = true;
  }
}

// ---------- Loop ----------
unsigned long lastWeatherFetch = 0;
const unsigned long WEATHER_INTERVAL = 1 * 60 * 1000;  // 10 minutes
static unsigned long lastDisplayUpdate = 0;

void loop() {
  static bool didInitialFetch = false;

  if (wifiJustConnected) {
    // give some time for wifi to get going
    delay(500);
    wifiJustConnected = false;
    display.println("Getting location");
    display.display();
    getLocation();
    display.println(city + "\nLat:" + String(latitude, 4) + " Lon:" + String(longitude, 4));
    display.display();

    syncTime();
    didInitialFetch = false;
  }

  if (wifiReady) {
    unsigned long now = millis();

    if (!didInitialFetch || now - lastWeatherFetch > WEATHER_INTERVAL) {
      fetchWeather();
      generateDryingScore(currentTempC, currentHumidity, currentWindMS, currentPrecipMM,
                          currentRadiation, currentEt0, hoursTillRain, dryingScore);
      lastWeatherFetch = now;
      didInitialFetch = true;

      String displayText = "drying score: " + String(dryingScore);

      Serial.println(dryingScore);
    }

    if (now - lastDisplayUpdate > 1000) {

      time_t now = time(nullptr);
      struct tm timeinfo;
      localtime_r(&now, &timeinfo);

      displayInformation(city, &timeinfo, currentTempC, currentHumidity, currentWindMS,
                         currentPrecipMM, currentRadiation, currentEt0, hoursTillRain, dryingScore);

      lastDisplayUpdate = now;
    }
  }

  checkButtonState();
  delay(50);
}
