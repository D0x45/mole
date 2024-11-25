// C99
#include "mole.h"

#include <stdint.h>

#if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#   error "TODO: implement bits.c for Big-Endian ISAs!"
#endif

inline uint32_t MoleReadU32LE(register char *buffer)
{
    register uint32_t a = 0;

    a  =  ((uint32_t)*buffer++)        & 0x000000FF;
    a |= (((uint32_t)*buffer++) << 8)  & 0x0000FF00;
    a |= (((uint32_t)*buffer++) << 16) & 0x00FF0000;
    a |= (((uint32_t)*buffer++) << 24) & 0xFF000000;

    return a;
}

inline uint16_t MoleReadU16LE(register char *buffer)
{
    register uint16_t a = 0;

    a  = ((uint16_t)*buffer++)         & 0x00FF;
    a |= (((uint16_t)*buffer++) << 8)  & 0xFF00;

    return a;
}
