
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <tock.h>
#include "firestorm.h"
#include "gpio.h"
#include "timer.h"
#include "mbedtls/md.h"
//#include "mbedtls/aes.h"

#define PIN 4

const mbedtls_md_info_t * md_info;
mbedtls_md_context_t md_context;
//mbedtls_aes_context aes_context;


void sha256(const unsigned char * in, size_t ilen, unsigned char * out) {
    // get parameters for hash
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    // clear context
    mbedtls_md_free(&md_context);
    // init context
    mbedtls_md_init(&md_context);
    // setup context
    mbedtls_md_setup(&md_context, md_info, 0);
    // start digest
    mbedtls_md_starts(&md_context);
    // update digest
    mbedtls_md_update(&md_context, in, ilen);
    // finish
    mbedtls_md_finish(&md_context, out);
}

void hmac_sha256(const unsigned char * in, size_t ilen, const unsigned char * key, size_t klen,  unsigned char * out) {
    // get parameters for hash
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    // clear context
    mbedtls_md_free(&md_context);
    // init context
    mbedtls_md_init(&md_context);
    // setup context
    mbedtls_md_setup(&md_context, md_info, 1);
    // start digest
    mbedtls_md_hmac_starts(&md_context, key, klen);
    // update digest
    mbedtls_md_hmac_update(&md_context, in, ilen);
    // finish
    mbedtls_md_hmac_finish(&md_context, out);
}

//void aes_128_cbc_enc(const unsigned char * in, size_t ilen, const unsigned char * key, unsigned char * iv, unsigned char * out) {
//    // clear context
//    mbedtls_aes_free(&aes_context);
//    // init context
//    mbedtls_aes_init(&aes_context);
//    // set key for operation
//    mbedtls_aes_setkey_enc(&aes_context, key, 128);
//    // set IV
//    mbedtls_aes_crypt_cbc(&aes_context, MBEDTLS_AES_ENCRYPT, ilen, iv, in, out);
//}

int main(void) {
    gpio_enable_output(PIN);
    gpio_clear(PIN);

    //printf("Test mbedtls\n");

    unsigned char key[16];
    unsigned char output[32];
    unsigned char message[256];

    // Generate psuedorandom key using hash
    sha256((const unsigned char *)"key", 3, output);
    memcpy(key, output, 16);

    // Generate pseudorandom message using hash
    for(int i = 0; i < 8; i++) {
      char tmp[32];
      itoa(i, tmp, 10);
      sha256((unsigned char *) tmp, 1, output);
      memcpy(message + i*32, output, 32);
    }


    while(1) {
      gpio_set(PIN);
      delay_ms(500);
      gpio_clear(PIN);
      delay_ms(500);

      // hash benchmarks
      printf("Test hash\n");
      gpio_set(PIN);
      sha256(message, 32, output);
      gpio_clear(PIN);
      delay_ms(100);
      gpio_set(PIN);
      sha256(message, 128, output);
      gpio_clear(PIN);
      delay_ms(100);
      gpio_set(PIN);
      sha256(message, 256, output);
      gpio_clear(PIN);
      delay_ms(500);

      // hmac benchmarks
      gpio_set(PIN);
      hmac_sha256(message, 32, key, 16, output);
      gpio_clear(PIN);
      delay_ms(100);
      gpio_set(PIN);
      hmac_sha256(message, 128, key, 16, output);
      gpio_clear(PIN);
      delay_ms(100);
      gpio_set(PIN);
      hmac_sha256(message, 256, key, 16, output);
      gpio_clear(PIN);
      delay_ms(500);

      unsigned char output32[512];
      unsigned char output128[512];
      unsigned char output256[512];

      // aes enc benchmarks
      //gpio_set(PIN);
      //aes_128_cbc_enc(message, 32, key, key, output32);
      //gpio_clear(PIN);
      //delay_ms(100);
      //gpio_set(PIN);
      //aes_128_cbc_enc(message, 128, key, key, output128);
      //gpio_clear(PIN);
      //delay_ms(100);
      //gpio_set(PIN);
      //aes_128_cbc_enc(message, 256, key, key, output256);
      //gpio_clear(PIN);
      //delay_ms(100);

      //gpio_set(PIN);
      //delay_ms(250);
      //gpio_clear(PIN);

      //printf("output: 0x");
      //for(int i = 0; i < 32; i++) {
      //  printf(" %x ", output[i]);
      //}
      //printf("\n");

    }
}
