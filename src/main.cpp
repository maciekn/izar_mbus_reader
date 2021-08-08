#include <Arduino.h>
#include "izar_wmbus.h"

extern uint32_t meterId;

IzarWmbus reader;


void setup() {
    Serial.begin(9600);
    reader.init(meterId);
}




void loop() {
    if(reader.fetchPacket()) {

    } else {
        delay(300);
    }
    

}