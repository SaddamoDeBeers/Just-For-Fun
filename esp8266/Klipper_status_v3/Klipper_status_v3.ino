#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Hardware definitions for 4 x 8x8 modules
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN D6  // GPIO12 on NodeMCU

MD_Parola matrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Wi-Fi credentials
const char* ssid = "wifi name";
const char* password = "wifi pass";

// Klipper printer IP and port
const char* printerIP = "192.168.1.35";
const int printerPort = 7125;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 10000; // 10 seconds

String scrollText = "Starting...";

String getURL(const String& endpoint) {
  return "http://" + String(printerIP) + ":" + String(printerPort) + endpoint;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println();  // blank line
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  matrix.begin();
  matrix.setIntensity(5); // Brightness (0-15)
  matrix.displayText(scrollText.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop() {
  if (matrix.displayAnimate()) {  // Animation finished, time to update text
    if (millis() - lastUpdate > updateInterval) {
      lastUpdate = millis();
      fetchPrintStatus();
      matrix.displayText(scrollText.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }
  }
}

void fetchPrintStatus() {
  HTTPClient http;
  WiFiClient client;
  String url = getURL("/printer/objects/query?print_stats");

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Received:");
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float progress = doc["result"]["status"]["print_stats"]["progress"] | -1;
      int time_remaining = doc["result"]["status"]["print_stats"]["time_remaining"] | -1;

      String newText = "";

      if (progress >= 0) {
        int percent = int(progress * 100);
        newText += String(percent) + "% ";
      } else {
        newText += "NoProg ";
      }

      if (time_remaining >= 0) {
        int hours = time_remaining / 3600;
        int minutes = (time_remaining % 3600) / 60;
        char timeStr[10];
        sprintf(timeStr, "%02d:%02d", hours, minutes);
        newText += String(timeStr);
      } else {
        newText += "NoTime";
      }

      scrollText = newText;
    } else {
      Serial.println("JSON parsing failed");
      scrollText = "JSON Error";
    }
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
    scrollText = "HTTP Error";
  }

  http.end();
}
