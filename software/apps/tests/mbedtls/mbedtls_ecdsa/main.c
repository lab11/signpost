/* Mbedtls benchmark for hash performance. Replace secp***.h file with curve
 * to test. Make sure to change app size in boards/<your_board>/src/main.rs and
 * tock/userland/linker.ld to accomodate larger app.
 * */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gpio.h>
#include <timer.h>
#include <tock.h>

#include "mbedtls/pk.h"
#include "mbedtls/md.h"
// Choose between curves defined in different .h files
#include "secp521r1.h"

const char* message = "hello!";

mbedtls_pk_context pk;
const mbedtls_pk_info_t * pk_info;
mbedtls_md_context_t md_context;
const mbedtls_md_info_t * md_info;
unsigned char sig[1024];
unsigned char hash[1024];

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

static int pseudorandom(void * data, unsigned char * output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = rand();
    }

    return 0;
}

int main(void) {
    size_t sig_len;
    int ret;

    srand(55);

    sha256(message, strlen(message), hash);
    printf("hash: 0x");
    for(int i = 0; i < 32; i++) {
      printf("%x", hash[i]);
    }
    printf("\n\n");

    while(1) {
      mbedtls_pk_free(&pk);
      mbedtls_pk_init(&pk);
      mbedtls_pk_parse_key(&pk, priv_key, strlen(priv_key)+1, NULL, 0);
      mbedtls_pk_parse_public_key(&pk, pub_key, strlen(pub_key)+1);
      delay_ms(1000);

      for(int i = 0; i < 5; i++) {
        printf("Test ECDSA sign\n");
        ret = mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, &sig_len, pseudorandom, NULL);
        printf("ret: %x\n", -ret);
      }

      printf("sig of len %d: 0x", sig_len);
      for(size_t i = 0; i < sig_len; i++) {
        printf("%x", sig[i]);
      }
      printf("\n\n");

      delay_ms(1000);

      for(int i = 0; i < 5; i++) {
        printf("Test ECDSA verify\n");
        ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, sig_len);
        printf("verify ret: %x\n", -ret);
      }

      delay_ms(1000);

    }
}
