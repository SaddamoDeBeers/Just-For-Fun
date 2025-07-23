#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Your Wifi";
const char* password = "Your Wifi Pass";

const char* printerIP = "192.168.1.35";  // Replace with your Klipper IP
const int printerPort = 7125;

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
}

void loop() {
  static unsigned long lastRequestTime = 0;
  const unsigned long interval = 10000; // 10 seconds

  if (millis() - lastRequestTime > interval) {
    lastRequestTime = millis();

    WiFiClient client;
    HTTPClient http;

    String url = getURL("/printer/objects/query?print_stats");
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Received payload:");
      Serial.println(payload);

      DynamicJsonDocument doc(2048);  // Increase size for bigger JSON
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float progress = doc["result"]["status"]["print_stats"]["progress"] | -1;
        int time_remaining = doc["result"]["status"]["print_stats"]["time_remaining"] | -1;

        if (progress >= 0) {
          Serial.printf("Print progress: %.1f%%\n", progress * 100);
        } else {
          Serial.println("Progress data not available");
        }

        if (time_remaining >= 0) {
          // Convert seconds to hh:mm:ss
          int hours = time_remaining / 3600;
          int minutes = (time_remaining % 3600) / 60;
          int seconds = time_remaining % 60;
          Serial.printf("Estimated time remaining: %02d:%02d:%02d\n", hours, minutes, seconds);
        } else {
          Serial.println("Estimated time remaining not available");
        }
      } else {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.printf("HTTP error: %d\n", httpCode);
    }

    http.end();
  }
}
