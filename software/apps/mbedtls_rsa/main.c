
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

const unsigned char * priv_key = "-----BEGIN PRIVATE KEY-----\n"
"MIIEwAIBADANBgkqhkiG9w0BAQEFAASCBKowggSmAgEAAoIBAQDuhe2pnbgahszc\n"
"k5vEUWP6hVXdTGprfyDJ/juY55ba5+1N0QqZV29FuocyO/WidznkF/qvpz13xVUy\n"
"daCqaqyqOQfP1MMxeHaSfN58ifgUK8LZZaGEWGDCN/+Glxx60j7igC6vOuqFLO1s\n"
"z56vPTv/6T1Zrzj0Qv/TXgMX1UmAG6mqCvH7i50XcFxxSIZHLMTSTx5+SWXoV/ax\n"
"CLD/WrIWOSfNy01VmrFwxWaJjBJoDcaiVui565D2108R5ibhJ2f9QZ8qEzVihJPb\n"
"5Ko9zqqrwYWwqfZcoCVtBz+bpPiHtC+6h1+dIFntilft9ulC3nAmvUX0KmK2HX2Z\n"
"fumF/+W3AgMBAAECggEBAMKCrSEY4T2gmAd4yBn8VY/ClBb3bgFjlpWioW87zKWP\n"
"ZNGEmyQZTUCLsByjENkAaVE8zM3ZkCLP5X5UtkTC4TOfdmNhmhwJpIgpFDZzPL8W\n"
"QLuQSXrnp2A60kIrkKoIMgf7iBaV8RHRZXzKgxlc0kULK2Crp8JpsL3peJRC9sFk\n"
"OoVJOBu+Ak7W5HXqk8osbuj+c4qOmkaaC2Mjb6OpmOMA6EJZWGQJO8NozNqKsyja\n"
"hWvw8EPgcKjPD1dywWIa2HDBgzBXj5ZU9fRzuo7Rg3Xu3z/DOOEWKD63mw7ioRCt\n"
"jzqzn0Sx2KJqd/QpFzINOxQmEqi0Yw3UuQOKG+kFMsECgYEA/HMAwssRjK8dOch8\n"
"7N0egCWZTGvGTwX4I4oIp2j7nc9kwESUP4NUfiR9oFT7r8+HpdjCM13TiVflmITG\n"
"ay+SXNhy4yYsuUDIpBiUxJP5bsHeP09FrVZj/85DqfaW9+XxNAQJPTlCsPHfNT0g\n"
"MYV+AeYAMfo2gY2Ny0uDZ62VIakCgYEA8eDIGA7vrdLyHHxi8Bd0cdQY4GFTuKX/\n"
"tR9hSBLA91xqKhe4pAf0FJ6NP34pH1hxrFa+BFWYXM0kEMPMGECwX300HI4IUnzM\n"
"1aQI1iaL72jQL4df6eRr9uwnJt4RR3OSqVxjOVLJ6y4yx1df45UedOAvQncJCJlI\n"
"NxV2xIb2KF8CgYEAxHfME81ubwqpuBqr/rtnzVt1nuARidafcykt6vvtNrf9NRUq\n"
"OvO+gh1sF6eL6Rud+hhjqw5OXwklCjnrIef4BIH/h4BiNhqRDASFYye2a9g9WxAa\n"
"rfgaAy9HwbLDc1JlEZJCjn9Nw9+5UvmrYF7/3gJeDIcqfFDqFVlDfyC8e2ECgYEA\n"
"4d0zPDQGYF0RNMR6ZxSf6gNSz4RS96QbKfkJkJHim5oykOfhxVqf8/kykM2wfNLM\n"
"I1BE1CtRwabWPMl5dlRB3ok99kQzRUPAyucIUWFhXsiTTy6sw+5HWyaeUM8hd4VW\n"
"uMvK5CaI4xIa3MachhojfKQzvW8Ggn8QTk++vjoA9yUCgYEAyjl7zISaqZMtsagl\n"
"92p43mD9WWchjKmSunLgnIhzDE8CZFtIwsmEgavfUK4iW/kwOonxvfYGRqTacemE\n"
"7ySJ8wkWs94XdZneMmTADD2tvtOumjPMrMzNxNXYh8BnY7betq7pH80coHg0bkuL\n"
"Ee7OeXq157aF4od/ch7LSx9t/Ps=\n"
"-----END PRIVATE KEY-----\n";

mbedtls_pk_context pk;
const mbedtls_pk_info_t * pk_info;
mbedtls_md_context_t md_context;
const mbedtls_md_info_t * md_info;

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
    unsigned char sig[384];
    unsigned char hash[32];
    size_t sig_len;
    int ret;

    srand(55);

    mbedtls_pk_init(&pk);
    mbedtls_pk_parse_key(&pk, priv_key, strlen(priv_key)+1, NULL, 0);

    gpio_enable_output(PIN);
    gpio_clear(PIN);

    sha256(message, strlen(message), hash);

    while(1) {
      delay_ms(250);
      printf("Test RSA\n");

      gpio_set(PIN);
      ret = mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, 0, sig, &sig_len, psuedorandom, NULL);
      gpio_clear(PIN);
      printf("ret: %x\n", ret);
      printf("sig of len %d: 0x", sig_len);

      for(int i = 0; i < sig_len; i++) {
        printf(" %x ", sig[i]);
      }
      printf("\n");

    }
}
