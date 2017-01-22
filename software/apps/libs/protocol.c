#include <string.h>
#include <stdbool.h>
#include "message.h"
#include "module.h"
#include "mbedtls/md.h"
#include "mbedtls/cipher.h"
#include "rng.h"

const mbedtls_md_info_t * md_info;
const mbedtls_cipher_info_t * cipher_info;
mbedtls_md_context_t md_context;
mbedtls_cipher_context_t cipher_context;

int cipher(const mbedtls_operation_t operation, uint8_t* key, uint8_t* in, size_t inlen, uint8_t* iv, uint8_t* out, size_t* olen) {
    uint8_t ivenc[16];
    int ret = 0;

    if (operation == MBEDTLS_ENCRYPT) {
        // Get 16 random bits for IV
        // TODO use mbedtls entropy
        rng_sync(iv, 16, 16);
        // copy to iv, to send with encrypted content
        memcpy(iv, ivenc, 16);
    }

    cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);
    mbedtls_cipher_init(&cipher_context);
    ret = mbedtls_cipher_setup(&cipher_context, cipher_info);
    if(ret<0) return ret;
    ret = mbedtls_cipher_setkey(&cipher_context, key, ECDH_KEY_LENGTH*8, operation);
    if(ret<0) return ret;
    ret = mbedtls_cipher_reset(&cipher_context);
    if(ret<0) return ret;
    ret = mbedtls_cipher_crypt(&cipher_context, iv, 16, in, inlen, out, olen);
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
                  size_t buflen, size_t len) {

    uint8_t* tempbuf[buflen];
    uint8_t* iv[16];
    size_t olen;
    // keep track of free location in buffer
    size_t idx=0;

    // if there isn't enough room in the provided buffer for an hmac/hash
    if (buflen - len < ECDH_KEY_LENGTH + 16) return -1;

    if(key!=NULL) {
        // encrypt buf and put into tempbuf
        cipher(MBEDTLS_ENCRYPT, key, buf, len, iv, tempbuf, &olen);
        // put iv in front of buf
        memcpy(buf, iv, 16);
        idx+=16;
        // hmac over encrypted content after iv
        message_digest(key, tempbuf, olen, buf+idx);
        idx+=32;
        // copy encrypted content to buf after iv and hmac
        memcpy(buf+idx, tempbuf, olen);
        idx+=olen;
    }
    else {
        memcpy(tempbuf, buf, len);
        message_digest(key, tempbuf, len, buf);
        idx+=32;
        memcpy(buf+idx, tempbuf, len);
        idx+=len;
    }


    // pass buffer to message
    //message_init(addr);
    //message_send(dest, buf, len+ECDH_KEY_LENGTH);
    return 0;
}

