// ===============================
// ESP32 BLE Wi-Fi Provisioning – Production-Ready Version
// ===============================
// Purpose:
//  - Provision Wi-Fi credentials over BLE once
//  - Persist credentials in NVS
//  - Reconnect automatically on reboot
//  - Provide a clean hook for application logic (e.g. weather fetching)
//
// Tested assumptions:
//  - ESP32 Arduino core ≥ 2.0.x
//  - 4 MB flash
//  - BLE provisioning via Espressif app
// ===============================

#include <WiFi.h>
#include <WiFiProv.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"
#include <WiFiUdp.h>
#include "qrcode.h"  // Arduino QRCode library
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------- OLED Display Configuration ------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1   // No reset pin
#define OLED_ADDR 0x3C  // Most common I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Provisioning Configuration ----------
const char *POP = "abcd1234";         // Proof-of-Possession PIN
const char *SERVICE_NAME = "PROV_1";  // BLE name (must start with PROV_)
const char *SERVICE_KEY = NULL;       // Not used for BLE

// Set true ONLY for factory reset builds
bool RESET_PROVISIONED = false;

// BLE service UUID (arbitrary but fixed)
uint8_t SERVICE_UUID[16] = {
  0xb4, 0xdf, 0x5a, 0x1c,
  0x3f, 0x6b, 0xf4, 0xbf,
  0xea, 0x4a, 0x82, 0x03,
  0x04, 0x90, 0x1a, 0x02
};

// ---------- State Tracking ----------
bool wifiReady = false;
volatile bool wifiJustConnected = false;

double latitude;
double longitude;
String city;
time_t nowEpoch;
struct tm *nowInfo;

bool oledReady = false;

// Weather snapshot (update these in fetchWeather())
double currentTempC = NAN;
double currentHumidity = NAN;
double currentWindMS = NAN;
double currentPrecipMM = NAN;

char timeBuf[24];

void initOLED() {
  Wire.begin();  // ESP32 default I2C pins: SDA=21, SCL=22

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
  oledReady = true;
}

// ---------- Event Handler ----------
void onWiFiEvent(arduino_event_t *event) {
  switch (event->event_id) {

    case ARDUINO_EVENT_PROV_START:
      Serial.println("[PROV] BLE provisioning started");
      break;

    case ARDUINO_EVENT_PROV_CRED_RECV:
      Serial.println("[PROV] Credentials received");
      break;

    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("[PROV] Provisioning successful");
      break;

    case ARDUINO_EVENT_PROV_CRED_FAIL:
      Serial.println("[PROV] Provisioning failed");
      if (event->event_info.prov_fail_reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) {
        Serial.println("[PROV] Wrong Wi-Fi password");
      } else {
        Serial.println("[PROV] Wi-Fi network not found");
      }
      break;

    case ARDUINO_EVENT_PROV_END:
      Serial.println("[PROV] Provisioning service stopped");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("[WIFI] Connected. IP: ");
      Serial.println(IPAddress(event->event_info.got_ip.ip_info.ip.addr));
      wifiReady = true;
      wifiJustConnected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("[WIFI] Disconnected, retrying...");
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

  // Start provisioning over BLE
  WiFiProv.beginProvision(
    NETWORK_PROV_SCHEME_BLE,
    NETWORK_PROV_SCHEME_HANDLER_FREE_BLE,
    NETWORK_PROV_SECURITY_1,
    POP,
    SERVICE_NAME,
    SERVICE_KEY,
    SERVICE_UUID,
    RESET_PROVISIONED);

  // Print ASCII QR code to Serial for testing
  Serial.println("[SYS] QR code for Serial Monitor (scan with phone):");
  WiFiProv.printQR(SERVICE_NAME, POP, "ble");  // optimized small QR
  Serial.println("[SYS] End of QR code");
}

void getLocation() {
  oledPrint("Getting location");
  HTTPClient http;
  http.begin("https://ipwho.is/");

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.println("[GEO] HTTP request failed");
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();


  // Parse only what we need to keep memory usage low
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.println("[GEO] JSON parse failed");
    return;
  }


  if (!doc["success"].as<bool>()) {
    Serial.println("[GEO] Geolocation lookup failed");
    return;
  }

  city = doc["city"] | "";
  latitude = doc["latitude"] | 0.0;
  longitude = doc["longitude"] | 0.0;

  Serial.printf("City: %s, Lat: %.4f, Lon: %.4f", city, latitude, longitude);
  String locationInfo = city + "\nlon: " + longitude + "lat: " + latitude;
  oledPrint(locationInfo);

  delay(2000);
}

void syncTime() {
  // Example: GMT (London/Bristol)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("[TIME] Waiting for NTP time sync");
  oledPrint("Waiting for NTP time sync");
  time_t nowSecs;
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
    oledPrint("Failed to sync time");
  } else {
    Serial.println("[TIME] Time synced successfully");
    oledPrint("Time synced");
  }

  delay(2000);
}

// Call this after wifiReady is true
void fetchWeather(double lat, double lon) {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WEATHER] Wi-Fi not connected");
    return;
  }

  // Construct URL using lat/lon
  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=" + String(lat, 6);
  url += "&longitude=" + String(lon, 6);
  url += "&hourly=temperature_2m,relative_humidity_2m,windspeed_10m,precipitation";
  url += "&daily=temperature_2m_max,temperature_2m_min";
  url += "&temperature_unit=celsius&timezone=auto";

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[WEATHER] HTTP GET failed, code: %d\n", httpCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<4096> doc;  // adjust size if needed
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.print("[WEATHER] JSON parse failed: ");
    Serial.println(err.c_str());
    return;
  }

  // Get current local hour
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[WEATHER] Failed to get local time");
    return;
  }
  int currentHour = timeinfo.tm_hour;

  // Access hourly data
  JsonArray times = doc["hourly"]["time"].as<JsonArray>();
  JsonArray temps = doc["hourly"]["temperature_2m"].as<JsonArray>();
  JsonArray winds = doc["hourly"]["windspeed_10m"].as<JsonArray>();
  JsonArray humidities = doc["hourly"]["relative_humidity_2m"].as<JsonArray>();
  JsonArray precip = doc["hourly"]["precipitation"].as<JsonArray>();

  Serial.println("[WEATHER] Next 5 hours:");
  // Ensure we don't go out of bounds
  for (size_t i = currentHour; i < currentHour + 5 && i < times.size(); i++) {
    Serial.printf("%s | %.1f°C | %.1f m/s\n",
                  times[i].as<const char *>(),
                  temps[i].as<double>(),
                  winds[i].as<double>(),
                  humidities[i].as<double>(),
                  precip[i].as<double>());
  }

  currentTempC = temps[currentHour].as<double>();
  currentHumidity = humidities[currentHour].as<double>();
  currentWindMS = winds[currentHour].as<double>();
  currentPrecipMM = precip[currentHour].as<double>();

  // Optional: store latest values to NVS for use elsewhere
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(300);

  // Oled needs to initialise before anything else
  initOLED();

  display.println("Init WiFi...");
  display.display();

  WiFi.onEvent(onWiFiEvent);

  if (WiFi.status() == WL_CONNECTED && !RESET_PROVISIONED) {
    Serial.println("[SYS] Already provisioned, connecting to Wi-Fi");
    WiFi.begin();
  } else {
    startProvisioning();
  }
}

unsigned long lastWeatherFetch = 0;
const unsigned long WEATHER_INTERVAL = 1 * 60 * 1000;  // 10 minutes

static unsigned long lastDisplayUpdate = 0;

void loop() {
  static bool didInitialFetch = false;

  if (wifiJustConnected) {
    wifiJustConnected = false;

    oledPrint("WiFi connected");
    getLocation();
    syncTime();
    didInitialFetch = false;
  }

  if (wifiReady) {
    unsigned long now = millis();
    
    if (!didInitialFetch || now - lastWeatherFetch > WEATHER_INTERVAL) {
      fetchWeather(latitude, longitude);
      lastWeatherFetch = now;
      didInitialFetch = true;
    }

    if(now - lastDisplayUpdate > 1000)
    {
      displayInformation();
      lastDisplayUpdate = now;
    }
  }
}

void oledPrint(const String &text) {
  if (!oledReady) return;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.println(text);
  display.display();
}

void displayInformation()
{
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  struct tm timeinfo;
  bool timeValid = getLocalTime(&timeinfo);

  char dateBuf[12];
  char timeBuf[8];

  if (timeValid) {
    strftime(dateBuf, sizeof(dateBuf), "%d/%m/%y", &timeinfo);
    strftime(timeBuf, sizeof(timeBuf), "%H:%M", &timeinfo);
  } else {
    strcpy(dateBuf, "--/--/--");
    strcpy(timeBuf, "--:--");
  }

  // -------- Top left: City --------
  display.setCursor(0, 0);
  String cityShort = city.length() > 10 ? city.substring(0, 10) : city;
  display.print(cityShort);

  // -------- Top right: Date --------
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(dateBuf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(128 - w, 0);
  display.print(dateBuf);

  // -------- Second row: Time (right-aligned) --------
  display.getTextBounds(timeBuf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(128 - w, 8);
  display.print(timeBuf);

  // Divider
  display.drawLine(0, 18, 127, 18, SSD1306_WHITE);

  // -------- Weather data --------
  int y = 22;

  display.setCursor(0, y);
  display.printf("Temp:  %.1f C", currentTempC);
  y += 10;

  display.setCursor(0, y);
  display.printf("Hum:   %.0f %%", currentHumidity);
  y += 10;

  float windMph = currentWindMS * 2.23694f;
  display.setCursor(0, y);
  display.printf("Wind:  %.1f mph", windMph);
  y += 10;

  display.setCursor(0, y);
  display.printf("Rain:  %.1f mm", currentPrecipMM);

  display.display();
} 



// ===============================
// Notes:
//  - Wi-Fi credentials are stored in NVS
//  - BLE is automatically freed after provisioning
//  - Reboot reconnects without provisioning
//  - Add a button to erase credentials if needed
// ===============================