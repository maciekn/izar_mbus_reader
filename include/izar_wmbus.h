#ifndef IZAR_WMBUS
#define IZAR_WMBUS
#include <Arduino.h>

enum FetchResult {
    FETCH_SUCCESSFUL,
    FETCH_NO_DATA,
    FETCH_OTHER_METER
};

class IzarWmbus {
    public:
        void init(uint32_t waterMeter);
        FetchResult fetchPacket();

    private:
        uint8_t ReceiveData2(byte *rxBuffer);
        uint32_t waterMeterId = 0;
};

#endif
