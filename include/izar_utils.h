#ifndef IZAR_UTILS
#define IZAR_UTILS

#include <Arduino.h>

#define GET_BIT(var, pos) ((var >> pos) & 0x01)

uint32_t uintFromBytes(uint8_t* data);
uint32_t uintFromBytesLittleEndian(uint8_t* data);
uint8_t decrypt(uint8_t* encoded, uint8_t len, uint8_t* decoded);
uint8_t decode3of6(uint8_t* encoded, uint8_t* decoded);


#endif
