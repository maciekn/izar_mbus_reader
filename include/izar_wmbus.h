#ifndef IZAR_WMBUS
#define IZAR_WMBUS
#include <Arduino.h>

enum FetchResult {
    FETCH_SUCCESSFUL,
    FETCH_NO_DATA,
    FETCH_OTHER_METER,
    FETCH_3OF6_ERROR,
    FETCH_CRC_ERROR
};

struct IzarResultData {
    uint32_t meterId;
    uint32_t waterUsage;
};

class IzarWmbus {
    public:
        void init(uint32_t waterMeter);
        FetchResult fetchPacket(IzarResultData* data);
        void ensureRx();

    private:
        uint8_t ReceiveData2(byte *rxBuffer);
        uint32_t waterMeterId = 0;
        const bool print_telegrams = 0;
        const bool print_decoded = 0;
        static int decode3outOf6(uint8_t* input, uint8_t inputLen, uint8_t* output, uint8_t& errors);
        bool checkCRCForSection(uint8_t* section, uint8_t sectionLen);
        bool checkCRC(uint8_t* packet, uint8_t len);

};

#endif
