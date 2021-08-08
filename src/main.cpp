#include <Arduino.h>
#include "izar_wmbus.h"

extern uint32_t meterId;

IzarWmbus reader;


void setup() {
    Serial.begin(9600);
    reader.init(meterId);
}


IzarResultData data;

void loop() {
    if(reader.fetchPacket(&data) == FETCH_SUCCESSFUL) {
        Serial.print("WatermeterId: ");
        Serial.println(data.meterId, HEX);

        Serial.print("Water consumption: ");
        Serial.println(data.waterUsage);
    } else {
        delay(300);
    }
    

}