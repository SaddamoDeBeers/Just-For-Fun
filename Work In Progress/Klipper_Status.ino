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

  WiFiClient client;
  HTTPClient http;

  String url = getURL("/printer/objects/query?print_stats");
  http.begin(client, url);  // yo yo
  int httpCode = http.GET();  // yo yo

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Received payload:");
    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float progress = doc["result"]["status"]["print_stats"]["progress"];
      Serial.printf("Print progress: %.1f%%\n", progress * 100);
    } else {
      Serial.println("JSON parsing failed");
    }
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
  }

  http.end();
}

void loop() {
  // You can add polling or display logic here
}
