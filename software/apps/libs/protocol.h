#pragma once

#include "module.h"

#define SHA256_LEN 32

int protocol_send(uint8_t addr, uint8_t dest,
                  uint8_t* key, uint8_t *buf,
                  size_t buflen, size_t len);
int protocol_recv(uint8_t* inbuf, size_t inlen,
                  uint8_t* key, uint8_t* appdata,
                  size_t* applen);
