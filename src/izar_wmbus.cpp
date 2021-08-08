
#include "izar_wmbus.h"

#include <ELECHOUSE_CC1101_SRC_DRV.h>

#include "izar_utils.h"
#include "wmbus_t_cc1101_config.h"

void IzarWmbus::init(uint32_t waterMeter) {
    if (waterMeter != 0) {
        waterMeterId = waterMeter;
    }

    if (ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("Connection OK");
    } else {
        Serial.println("Connection Error");
    }
    ELECHOUSE_cc1101.Init();

    for (uint8_t i = 0; i < WMBUS_T_CC1101_CONFIG_LEN; i++) {
        ELECHOUSE_cc1101.SpiWriteReg(WMBUS_T_CC1101_CONFIG_BYTES[i << 1],
                                     WMBUS_T_CC1101_CONFIG_BYTES[(i << 1) + 1]);
    }

    ELECHOUSE_cc1101.SpiStrobe(CC1101_SCAL);

    if (ELECHOUSE_cc1101.SpiReadStatus(CC1101_VERSION) != 4) {
        Serial.println(
            "WARNING! CC1101_VERSION should be equal 4! Is there any "
            "connection issue?");
    }

    ELECHOUSE_cc1101.SetRx();

    Serial.println("device initialized");
}

uint8_t IzarWmbus::ReceiveData2(byte* rxBuffer) {
    uint8_t size = ELECHOUSE_cc1101.SpiReadStatus(CC1101_RXBYTES) & 0x7F;
    if (size) {
        ELECHOUSE_cc1101.SpiReadBurstReg(CC1101_RXFIFO, rxBuffer, size);
    }
    ELECHOUSE_cc1101.SpiStrobe(CC1101_SFRX);
    ELECHOUSE_cc1101.SpiStrobe(CC1101_SRX);
    return size;
}

uint8_t buffer[128] = {0};
uint8_t decoded[64] = {0};
uint8_t decrypted[64] = {0};

inline void dumpHex(uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

FetchResult IzarWmbus::fetchPacket(IzarResultData* data) {
    if (ELECHOUSE_cc1101.CheckRxFifo(0)) {
        //====READ====
        uint8_t len = ReceiveData2(buffer);
        uint8_t decodeErrors = 0;


        //====DECODE====
        int decodedLen = decode3outOf6(buffer, len, decoded, decodeErrors);

        if (decodeErrors != 0) {
            return FETCH_3OF6_ERROR;
        }

        uint32_t thisMeterId = uintFromBytesLittleEndian(decoded + 4);

        if (waterMeterId != 0) {
            if (thisMeterId != waterMeterId) {
                return FETCH_OTHER_METER;
            }
        } else {
            Serial.print("Meter ID: ");
            Serial.println(thisMeterId, HEX);
        }

        data->meterId = thisMeterId;

        // for some reason decoded[10] and [11] are no part of the frame
        // TODO - find why!!!
        for (int i = 12; i < decodedLen; i++) {
            decoded[i - 2] = decoded[i];
        }
        decoded[decodedLen - 1] = 0;
        decoded[decodedLen] = 0;
        decodedLen -= 2;

        //====DECRYPT====
        uint8_t decryptedLen = decrypt(decoded, decodedLen, decrypted);

        if (print_telegrams) {
            dumpHex(decoded, decodedLen);
        }
        if (print_decoded) {
            dumpHex(decrypted, decryptedLen);
        }

        data->waterUsage = uintFromBytesLittleEndian(decrypted + 1);

        return FETCH_SUCCESSFUL;
    } else {
        return FETCH_NO_DATA;
    }
}

int IzarWmbus::decode3outOf6(uint8_t* input, const uint8_t inputLen,
                             uint8_t* output, uint8_t& errors) {
    int i = 0;
    errors = 0;
    for (i = 0; i < inputLen / 3; i++) {
        if (decode3of6Single(buffer + (i * 3), decoded + (i * 2)) == -1) {
            errors++;
        }
    }
    return i * 2;
}
