
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 32

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "eir59735987";
const char* password = "bbnjgf7PBM";
const char* server_host = "http://192.168.1.63/submit";

const char* room_id = "room1"; // Change to "room2" for the second ESP32

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int air_ppm = analogRead(MQ135_PIN);

  if (!isnan(temperature) && !isnan(humidity)) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String url = String(server_host)
                   + "?id=" + room_id
                   + "&temp=" + String(temperature)
                   + "&hum=" + String(humidity)
                   + "&air=" + String(air_ppm);

      http.begin(url);
      int code = http.GET();
      http.end();

      Serial.printf("Sent Temp: %.1f, Hum: %.1f, Air: %.1f ppm\n",
                    temperature, humidity, air_ppm);
    }
  } else {
    Serial.println("Sensor read failed");
  }

  delay(10000);
}
