#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "message.h"
#include "protocol.h"
#include "app.h"
#include "module.h"
#include "mbedtls/md.h"
#include "mbedtls/cipher.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "rng.h"

static const mbedtls_md_info_t * md_info;
static mbedtls_md_context_t md_context;
static const mbedtls_cipher_info_t * cipher_info;
static mbedtls_cipher_context_t cipher_context;
static mbedtls_entropy_context entropy_context;
static mbedtls_ctr_drbg_context ctr_drbg_context;

int rng_wrapper(void* data __attribute__ ((unused)), uint8_t* out, size_t len, size_t* olen) {
    int num = rng_sync(out, len, len);
    if (num < 0) return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    *olen = num;
    return 0;
}

int cipher(const mbedtls_operation_t operation, uint8_t* key, uint8_t* in, size_t inlen, uint8_t* iv, uint8_t* out, size_t* olen) {
    uint8_t ivenc[MBEDTLS_MAX_IV_LENGTH];
    int ret = 0;

    if (operation == MBEDTLS_ENCRYPT) {
        // Get 16 random bits for IV
        // TODO make entropy a global resource
        mbedtls_entropy_init(&entropy_context);
        mbedtls_entropy_add_source(&entropy_context, rng_wrapper, NULL, 48, true);
        uint8_t start[32];
        rng_sync(start, 32, 32);
        mbedtls_ctr_drbg_free(&ctr_drbg_context);
        mbedtls_ctr_drbg_init(&ctr_drbg_context);
        ret = mbedtls_ctr_drbg_seed(&ctr_drbg_context, mbedtls_entropy_func, &entropy_context, start, 32);
        if (ret < 0) return ret;
        mbedtls_ctr_drbg_set_prediction_resistance(&ctr_drbg_context, MBEDTLS_CTR_DRBG_PR_ON);
        mbedtls_ctr_drbg_random(&ctr_drbg_context, ivenc, MBEDTLS_MAX_IV_LENGTH);
        // copy to iv, to send with encrypted content
        memcpy(iv, ivenc, MBEDTLS_MAX_IV_LENGTH);
    }

    //reset cipher ctx
    mbedtls_cipher_free(&cipher_context);
    cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CTR);
    mbedtls_cipher_init(&cipher_context);
    ret = mbedtls_cipher_setup(&cipher_context, cipher_info);
    if(ret<0) return ret;
    ret = mbedtls_cipher_setkey(&cipher_context, key, ECDH_KEY_LENGTH*8, operation);
    if(ret<0) return ret;
    ret = mbedtls_cipher_reset(&cipher_context);
    if(ret<0) return ret;
    //encrypt/decrypt
    ret = mbedtls_cipher_crypt(&cipher_context, iv, MBEDTLS_MAX_IV_LENGTH, in, inlen, out, olen);
    if(ret<0) return ret;


    return 0;
}

int message_digest(uint8_t* key, uint8_t* in, size_t inlen, uint8_t* out) {
    int ret = 0;

    // setup mbedtls message digest
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_init(&md_context);
    ret = mbedtls_md_setup(&md_context, md_info, (key!=NULL));
    if(ret<0) return ret;

    // switch on performing hmac or hash
    if(key) {
        ret = mbedtls_md_hmac_starts(&md_context, key, ECDH_KEY_LENGTH);
        if(ret<0) return ret;
        ret = mbedtls_md_hmac_update(&md_context, in, inlen);
        if(ret<0) return ret;
        ret = mbedtls_md_hmac_finish(&md_context, out);
        if(ret<0) return ret;
    }
    else {
        ret = mbedtls_md_starts(&md_context);
        if(ret<0) return ret;
        ret = mbedtls_md_update(&md_context, in, inlen);
        if(ret<0) return ret;
        ret = mbedtls_md_finish(&md_context, out);
        if(ret<0) return ret;
    }

    mbedtls_md_free(&md_context);

    return ret;
}

int protocol_send(uint8_t dest, uint8_t* key,
                  uint8_t *buf, size_t len) {

    // len is too large
    if(len > BUFSIZE) return -1;
    // tempbuf stores encrypted buf, needs to be multiple of 16
    uint8_t tempbuf[16*(len/16+(len%16 != 0))];
    // sendbuf is what is sent to message layer, needs to fit hash and IV
    uint8_t sendbuf[len+MBEDTLS_MAX_IV_LENGTH+SHA256_LEN];
    uint8_t iv[MBEDTLS_MAX_IV_LENGTH];
    size_t olen=0;
    // keep track of free location in buffer
    size_t size=0;

    if(key!=NULL) {
        // encrypt buf and put into tempbuf
        cipher(MBEDTLS_ENCRYPT, key, buf, len, iv, tempbuf, &olen);
        // put iv in front of sendbuf
        memcpy(sendbuf, iv, MBEDTLS_MAX_IV_LENGTH);
        size+=MBEDTLS_MAX_IV_LENGTH;
        // copy encrypted content to sendbuf after iv
        memcpy(sendbuf+size, tempbuf, olen);
        size+=olen;
        // hmac over iv and content
        message_digest(key, sendbuf, size, sendbuf+size);
        size+=SHA256_LEN;
    }
    // otherwise just hash over content
    else {
        memcpy(sendbuf, buf, len);
        size += len;
        message_digest(key, sendbuf, len, sendbuf+len);
        size += SHA256_LEN;
    }

    // pass buffer to message
    // expects message_init to have been called by module_init
    return message_send(dest, sendbuf, size);
}

int protocol_recv(uint8_t* buf, size_t buflen, size_t len, uint8_t* key, size_t* olen) {

    // len is too large
    if(len > BUFSIZE) return -1;
    uint8_t h[SHA256_LEN];
    uint8_t temp[len];
    int result = 0;

    if (len > buflen || len < SHA256_LEN) return -1;
    // check hmac/hash
    message_digest(key, buf-SHA256_LEN, SHA256_LEN, h);
    if (!memcmp(h, buf+len-SHA256_LEN, SHA256_LEN)) return -1;
    //printf("encrypted: 0x");
    //for(int i = 0; i < len; i++) {
    //    printf("%02x", buf[i]);
    //}
    //printf("\n");
    // decrypt if needed
    if(key != NULL) {
        result = cipher(MBEDTLS_DECRYPT, key, buf+MBEDTLS_MAX_IV_LENGTH, len-SHA256_LEN-MBEDTLS_MAX_IV_LENGTH, buf, temp, olen);
    }
    memcpy(buf, temp, *olen);

    return result;
}
