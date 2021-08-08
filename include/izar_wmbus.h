#ifndef IZAR_WMBUS
#define IZAR_WMBUS
#include <Arduino.h>

class IzarWmbus {
    public:
        void init();
        bool fetchPacket();

    private:
        uint8_t ReceiveData2(byte *rxBuffer);
};

#endif
