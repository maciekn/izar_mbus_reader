
#include "izar_wmbus.h"

#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "wmbus_t_cc1101_config.h"
#include "izar_utils.h"


void IzarWmbus::init() {
    if (ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("Connection OK");
    } else {
        Serial.println("Connection Error");
    }
    ELECHOUSE_cc1101.Init();

    for(uint8_t i=0; i<WMBUS_T_CC1101_CONFIG_LEN; i++) {
        ELECHOUSE_cc1101.SpiWriteReg(WMBUS_T_CC1101_CONFIG_BYTES[i<<1], WMBUS_T_CC1101_CONFIG_BYTES[(i<<1) + 1]);
    }
  
    ELECHOUSE_cc1101.SpiStrobe(CC1101_SCAL);


    Serial.print("CC1101_PARTNUM ");  // cc1101=0
    Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_PARTNUM));
    Serial.print("CC1101_VERSION ");  // cc1101=4
    Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_VERSION));
    Serial.print("CC1101_MARCSTATE ");
    Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_MARCSTATE) & 0x1f);

    ELECHOUSE_cc1101.SetRx();

    Serial.println("device initialized");
}


uint8_t IzarWmbus::ReceiveData2(byte *rxBuffer){
    uint8_t size = ELECHOUSE_cc1101.SpiReadStatus(CC1101_RXBYTES) & 0x7F;
    if(size) {
        ELECHOUSE_cc1101.SpiReadBurstReg(CC1101_RXFIFO,rxBuffer,size);
    }
    ELECHOUSE_cc1101.SpiStrobe(CC1101_SFRX);
    ELECHOUSE_cc1101.SpiStrobe(CC1101_SRX);
    return size;
}


uint8_t buffer[500] = {0};
uint8_t decoded[291];


bool IzarWmbus::fetchPacket() {
    if (ELECHOUSE_cc1101.CheckRxFifo(0)) {
        
        uint8_t len = ReceiveData2(buffer);
        bool print_telegrams= 0;
        bool print_decoded = 0;

        int MBPlen = 0;
        int decodeerrors = 0;
        for(int i=0; i<=len-3; i+=3){
            if(decode3of6(buffer+i, decoded+MBPlen) == -1){
                decodeerrors++;
            }
            MBPlen+=2;
        }


        // for(int i=0; i<4;i++){
        //     if(meterId[i] != decoded[7-i]) {
        //         //not our meter, escape!
        //         return;
        //     }
        // }

        Serial.print("Meter ID");
        Serial.println(uintFromBytesLittleEndian(decoded + 4), HEX);


        //for some reason decoded[10] and [11] are no part of the frame
        //TODO - find why!!!
        uint8_t frame[30];
        int framelen = 0;
        for(int i=0; i<MBPlen; i++){
            if(i != 10 && i!= 11) {
                frame[framelen++] = decoded[i];
            }
        }

        Serial.println(framelen);
        
        // Serial.print("framelen: ");
        // Serial.println(framelen);
        if(!decodeerrors) {
            uint8_t decrypted[30] = {0};
            uint8_t decrypted_size = decrypt(frame, framelen, decrypted);
            if(print_telegrams){
                for (int i=0; i<MBPlen; i++){
                    Serial.print(decoded[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
            if(print_decoded) {
                for (int i=0; i<decrypted_size; i++){
                    Serial.print(decrypted[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
            Serial.println(uintFromBytesLittleEndian(decrypted + 1));
        } else {
            Serial.print("Decoding went wrong!: ");
            Serial.println(decodeerrors);
            
        }


        return true;
    } else {
        return false;
    }
}