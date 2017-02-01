#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "message.h"
#include "protocol.h"
#include "module.h"
#include "mbedtls/md.h"
#include "mbedtls/cipher.h"
#include "rng.h"

const mbedtls_md_info_t * md_info;
mbedtls_md_context_t md_context;
const mbedtls_cipher_info_t * cipher_info;
mbedtls_cipher_context_t cipher_context;

int cipher(const mbedtls_operation_t operation, uint8_t* key, uint8_t* in, size_t inlen, uint8_t* iv, uint8_t* out, size_t* olen) {
    uint8_t ivenc[MBEDTLS_MAX_IV_LENGTH];
    int ret = 0;

    if (operation == MBEDTLS_ENCRYPT) {
        // Get 16 random bits for IV
        // TODO use mbedtls entropy
        rng_sync(ivenc, MBEDTLS_MAX_IV_LENGTH, MBEDTLS_MAX_IV_LENGTH);
        // copy to iv, to send with encrypted content
        memcpy(iv, ivenc, MBEDTLS_MAX_IV_LENGTH);
    }

    cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CTR);
    mbedtls_cipher_init(&cipher_context);
    ret = mbedtls_cipher_setup(&cipher_context, cipher_info);
    if(ret<0) return ret;
    ret = mbedtls_cipher_setkey(&cipher_context, key, ECDH_KEY_LENGTH*8, operation);
    if(ret<0) return ret;
    ret = mbedtls_cipher_reset(&cipher_context);
    if(ret<0) return ret;
    ret = mbedtls_cipher_crypt(&cipher_context, iv, MBEDTLS_MAX_IV_LENGTH, in, inlen, out, olen);
    if(ret<0) return ret;

    mbedtls_cipher_free(&cipher_context);

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

int protocol_send(uint8_t addr, uint8_t dest,
                  uint8_t* key, uint8_t *buf,
                  size_t len) {

    // tempbuf stores encrypted buf, needs to be multiple of 16
    uint8_t tempbuf[16*(len/16+(len%16 != 0))];
    // sendbuf is what is sent to message layer, needs to fit hash and IV
    uint8_t sendbuf[len+MBEDTLS_MAX_IV_LENGTH+SHA256_LEN];
    uint8_t iv[MBEDTLS_MAX_IV_LENGTH];
    size_t olen;
    size_t size;
    // keep track of free location in buffer
    size=0;

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
    // TODO init should be handled in different function
    message_init(addr);
    message_send(dest, sendbuf, size);

    // testing:
    //printf("(%d) protocol buf:\n", size);
    //for(int i = 0; i < size; i++) {
    //    printf("%02x", sendbuf[i]);
    //}
    //printf("\n");
    //protocol_recv(sendbuf, size, key, buf, &olen);
    //printf("(%d) decrypt buf:\n", olen);
    //for(int i = 0; i < olen; i++) {
    //    printf("%02x", buf[i]);
    //}
    //printf("\n");

    return 0;
}

int protocol_recv(uint8_t* inbuf, size_t inlen, uint8_t* key, uint8_t* appdata, size_t* applen) {
    uint8_t h[SHA256_LEN];

    // check hmac/hash
    message_digest(key, inbuf-SHA256_LEN, SHA256_LEN, h);
    if (!memcmp(h, inbuf+inlen-SHA256_LEN, SHA256_LEN)) return -1;
    // decrypt if needed
    if(key != NULL) {
        return cipher(MBEDTLS_DECRYPT, key, inbuf+MBEDTLS_MAX_IV_LENGTH, inlen-SHA256_LEN-MBEDTLS_MAX_IV_LENGTH, inbuf, appdata, applen);
    }
    return 0;
}
