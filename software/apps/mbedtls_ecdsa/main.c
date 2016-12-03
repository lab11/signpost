
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
#include "mbedtls/pk.h"
#include "mbedtls/md.h"

#define PIN 4

const unsigned char * message = "hello!";

const unsigned char * priv_key = "-----BEGIN EC PRIVATE KEY-----\n"
"MIGkAgEBBDCKP1AhGp+y3yg62oHT2oj+/BjoL1AuymAkmmAHEeRWNu4HohetfOGc\n"
"EcapFVxCgrmgBwYFK4EEACKhZANiAAQhBuuTDJznKmErsQyogMJttJShr8Z0jxe9\n"
"OzcE4kBzxfrbpUdxPUt6GZ+i93FuZhjd9s58iibV6gv94CzjiO013SV2fZbTbPH5\n"
"/WWO9etXjlQQkf1clga4gHLGWHPMlkE=\n"
"-----END EC PRIVATE KEY-----\n";

const unsigned char * pub_key = "-----BEGIN PUBLIC KEY-----\n"
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEIQbrkwyc5yphK7EMqIDCbbSUoa/GdI8X\n"
"vTs3BOJAc8X626VHcT1LehmfovdxbmYY3fbOfIom1eoL/eAs44jtNd0ldn2W02zx\n"
"+f1ljvXrV45UEJH9XJYGuIByxlhzzJZB\n"
"-----END PUBLIC KEY-----\n";

mbedtls_pk_context pk;
const mbedtls_pk_info_t * pk_info;
mbedtls_md_context_t md_context;
const mbedtls_md_info_t * md_info;
unsigned char sig[1024];
unsigned char hash[1024];

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

int psuedorandom(void * data, unsigned char * output, size_t len) {
    for (int i = 0; i < len; i++) {
        output[i] = rand();
    }

    return 0;
}

int main(void) {
    size_t sig_len;
    int ret;

    srand(55);


    gpio_enable_output(PIN);
    gpio_clear(PIN);

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

      gpio_set(PIN);
      delay_ms(250);
      gpio_clear(PIN);
      delay_ms(250);

      printf("Test ECDSA sign\n");

      gpio_set(PIN);
      ret = mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, &sig_len, psuedorandom, NULL);
      gpio_clear(PIN);
      printf("ret: %x\n", ret);

      printf("sig of len %d: 0x", sig_len);
      for(int i = 0; i < sig_len; i++) {
        printf("%x", sig[i]);
      }
      printf("\n\n");

      delay_ms(250);

      printf("Test ECDSA verify\n");

      gpio_set(PIN);
      ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, sig_len);
      gpio_clear(PIN);
      printf("verify ret: %x\n", -ret);

      delay_ms(250);

    }
}
