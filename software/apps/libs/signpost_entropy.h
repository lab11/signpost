#pragma once

#include "mbedtls/ctr_drbg.h"

extern mbedtls_ctr_drbg_context ctr_drbg_context;

int signpost_entropy_init (void);
int signpost_entropy_rand(uint8_t* buf, size_t len);
