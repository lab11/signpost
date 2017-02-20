#include "crc.h"
#include <stdint.h>

uint16_t computeCRC16(const uint8_t *data, uint32_t lengthInBytes)
{
    uint32_t crc = 0x1d0f;

    uint32_t j;
    for (j = 0; j < lengthInBytes; ++j)
    {
        uint32_t i;
        uint32_t byte = data[j];
        crc ^= byte << 8;
        for (i = 0; i < 8; ++i)
        {
            uint32_t temp = crc << 1;
            if (crc & 0x8000)
            {
                temp ^= 0x1021;
            }
            crc = temp;
        }
    }

    return crc;
}
