#pragma once

#include "module.h"

int protocol_send(uint8_t addr, uint8_t dest,
                  uint8_t* key, uint8_t *buf,
                  size_t buflen, size_t len);

int message_digest(uint8_t* key, uint8_t* in, size_t inlen, uint8_t* out);
