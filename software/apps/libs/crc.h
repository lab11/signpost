#ifndef _CRC_H
#define _CRC_H

#include <stdint.h>
#include "tock.h"

//! This implementation is slow but small in size.
uint16_t computeCRC16(const uint8_t *data, uint32_t lengthInBytes);

#endif
