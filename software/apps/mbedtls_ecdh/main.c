/* Mbedtls benchmark for hash performance. Make sure to change app size in
 * boards/<your_board>/src/main.rs and tock/userland/linker.ld to accomodate
 * larger app.
 * */

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
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/md.h"

mbedtls_ecdh_context ecdh_0;
mbedtls_ecdh_context ecdh_1;

unsigned char buf[256];
unsigned char buf2[256];

static int pseudorandom(void * data, unsigned char * output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = rand();
    }

    return 0;
}


int main(void) {
    int ret = 0;
    srand(55);


    size_t olen;
    size_t olen2;
    while(1) {
      olen = 0;
      memset(buf, 0, 256);
      mbedtls_ecdh_free(&ecdh_0);
      mbedtls_ecdh_free(&ecdh_1);
      mbedtls_ecdh_init(&ecdh_0);
      mbedtls_ecdh_init(&ecdh_1);
      mbedtls_ecp_group_load(&ecdh_0.grp, MBEDTLS_ECP_DP_SECP192R1);
      mbedtls_ecp_group_load(&ecdh_1.grp, MBEDTLS_ECP_DP_SECP192R1);

      delay_ms(2000);

      printf("\nTest ECDH handshake\n");

      printf("Server generate params\n");
      // server generate params
      delay_ms(250);
      ret = mbedtls_ecdh_make_params(&ecdh_1, &olen, buf, 256, pseudorandom, NULL);
      delay_ms(250);
      printf("ret make params = %x\n", -ret);
      printf("client read params\n");
      // client read params and make public params
      const unsigned char * buf_ptr = buf;
      ret = mbedtls_ecdh_read_params(&ecdh_0, &buf_ptr, buf_ptr+olen);
      printf("ret read params = %x\n", -ret);
      printf("client make public\n");
      ret = mbedtls_ecdh_make_public(&ecdh_0, &olen, buf, 256, pseudorandom, NULL);
      ret = mbedtls_ecdh_calc_secret(&ecdh_0, &olen2, buf2, 256, pseudorandom, NULL);
      printf("ret make public = %x\n", -ret);
      // server read public and calc secret
      printf("server read and calc\n");
      ret = mbedtls_ecdh_read_public(&ecdh_1, buf, olen);
      delay_ms(250);
      for(size_t i = 0; i < 5; i++)
        ret = mbedtls_ecdh_calc_secret(&ecdh_1, &olen, buf, 256, pseudorandom, NULL);
      delay_ms(250);

      printf("server secret key: 0x");
      for(size_t i = 0; i < olen; i++) {
        printf("%x", buf[i]);
      }
      printf("\n");
      printf("client secret key: 0x");
      for(size_t i = 0; i < olen2; i++) {
        printf("%x", buf2[i]);
      }
      printf("\n");
      printf("\n");
    }
}
