#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#include "izar_wmbus.h"

extern uint32_t meterId;
extern const char* ssid;
extern const char* password;
extern const char* mqttServer;
extern const int mqttPort;

IzarWmbus reader;

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    // reader.init(meterId);
    reader.init(0);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqttServer, mqttPort);
}

IzarResultData data;
StaticJsonDocument<200> doc;

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    FetchResult result = reader.fetchPacket(&data);
    if (result == FETCH_SUCCESSFUL) {
        doc["meter"] = data.meterId;
        doc["usg"] = data.waterUsage;
        char buffer[256];
        size_t n = serializeJson(doc, buffer);
        client.publish("water/consumption", buffer, n);
        
        Serial.print("WatermeterId: ");
        Serial.println(data.meterId, HEX);

        Serial.print("Water consumption: ");
        Serial.println(data.waterUsage);
    } else {
        delay(300);
        reader.ensureRx(); 
    }
}