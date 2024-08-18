#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <MQ135.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MQ135PIN 34
MQ135 mq135(MQ135PIN);

const char* ssid = "Marco";
const char* password = "Forza@123";
const char* mqtt_server = "demo.thingsboard.io";
const char* access_token = "jmesdltswi6cfg9a4jao";

// Initialize the WifiClient and PubSubClient
WiFiClient espClient;
PubSubClient client(espClient);

// Connect to WiFi network
void setup_wifi() {
    delay(10);
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// Reconnect to the MQTT server
void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.println("Connecting to MQTT server...");
        String clientID = "ESP32Client-";
        clientID += String(random(0xffff), HEX);
        if (client.connect(clientID.c_str(), access_token, NULL)) {
            Serial.println("Connected to the MQTT server");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    float co2 = mq135.getPPM();

    if (isnan(humidity) || isnan(temperature) || isnan(co2)) {
        Serial.println("Failed to read from DHT sensor or MQ135 sensor");
        return;
    }

    String payload = "{\"temperature\": " + String(temperature) +
                     ", \"humidity\": " + String(humidity) +
                     ", \"co2\": " + String(co2) + "}";

    // Publish the payload to the MQTT server
    if (client.publish("v1/devices/me/telemetry", payload.c_str())) {
        Serial.println("Data sent successfully: " + payload);
    } else {
        Serial.println("Failed to send data");
    }

    delay(30000);
}