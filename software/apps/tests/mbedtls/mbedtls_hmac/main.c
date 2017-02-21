/* Mbedtls benchmark for hmac performance. Replace block size with size in bytes
 * to test. Make sure to change app size in boards/<your_board>/src/main.rs and
 * tock/userland/linker.ld to accomodate larger app.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <tock.h>
#include "firestorm.h"
#include "timer.h"
#include "mbedtls/md.h"
#include "mbedtls/aes.h"

#define PIN 4
#define BLOCK_SIZE 8192

const mbedtls_md_info_t * md_info;
mbedtls_md_context_t md_context;
mbedtls_aes_context aes_context;

unsigned char message[8192];
unsigned char output[8192];

static void sha256(const unsigned char * in, size_t ilen, unsigned char * out) {
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

int main(void) {
    printf("\n\nTest mbedtls\n");

    unsigned char key[32];
    int ret;
    // Generate psuedorandom key using hash
    sha256((const unsigned char *)"key", 3, key);

    // Generate pseudorandom message using hash
    for(int i = 0; i < 8192/32; i++) {
      char tmp[32];
      itoa(i, tmp, 10);
      sha256((unsigned char *) tmp, 1, output);
      memcpy(message + i*32, output, 32);
    }

    //printf("message: 0x");
    //for(int i = 0; i < 16384; i++) {
    //  printf("%x", message[i]);
    //}
    //printf("\n");

    delay_ms(1000);

    while(1) {
      // hmac benchmarks
      // get parameters for hash
      md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
      // clear context
      mbedtls_md_free(&md_context);
      // init context
      mbedtls_md_init(&md_context);
      // setup context
      ret = mbedtls_md_setup(&md_context, md_info, 1);
      printf("ret = %x\n", -ret);
      delay_ms(500);
      // start digest
      mbedtls_md_hmac_starts(&md_context, key, 32);
      for(int i = 0; i < 2*16384/(BLOCK_SIZE/16); i++) {
        // update digest
        ret = mbedtls_md_hmac_update(&md_context, message, BLOCK_SIZE);
        if(ret) printf("ret = %x", -ret);
      }
      // finish
      ret = mbedtls_md_hmac_finish(&md_context, output);
      printf("ret = %x\n", -ret);

      printf("output: 0x");
      for(int i = 0; i < 32; i++) {
        printf(" %x ", output[i]);
      }
      printf("\n");

    }
}
