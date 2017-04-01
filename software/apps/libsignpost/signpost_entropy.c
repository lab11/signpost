#include <stdbool.h>

#include "mbedtls/entropy.h"

#include "rng.h"
#include "signpost_entropy.h"

mbedtls_ctr_drbg_context ctr_drbg_context;
static mbedtls_entropy_context entropy_context;
static uint8_t drbg_data[32];

static int rng_wrapper(void* data __attribute__ ((unused)), uint8_t* out, size_t len, size_t* olen) {
    int num = rng_sync(out, len, len);
    if (num < 0) return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    *olen = num;
    return 0;
}

int signpost_entropy_init (void) {
    int ret;
    // random start seed
    ret = rng_sync(drbg_data, 32, 32);
    if (ret < 0) return ret;

    // init entropy and prng
    mbedtls_entropy_init(&entropy_context);
    ret = mbedtls_entropy_add_source(&entropy_context, rng_wrapper, NULL, 48, true);
    if (ret < 0) return ret;
    mbedtls_ctr_drbg_free(&ctr_drbg_context);
    mbedtls_ctr_drbg_init(&ctr_drbg_context);
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg_context, mbedtls_entropy_func, &entropy_context, drbg_data, 32);
    if (ret < 0) return ret;
    mbedtls_ctr_drbg_set_prediction_resistance(&ctr_drbg_context, MBEDTLS_CTR_DRBG_PR_ON);
    return 0;
}

int signpost_entropy_rand(uint8_t* buf, size_t len) {
    return mbedtls_ctr_drbg_random(&ctr_drbg_context, buf, len);
}
