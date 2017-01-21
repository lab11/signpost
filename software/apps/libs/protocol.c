#include <string.h>
#include <stdbool.h>
#include "message.h"
#include "module.h"
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "rng.h"

const mbedtls_md_info_t * md_info;
mbedtls_md_context_t md_context;
mbedtls_aes_context aes_context;

int protocol_send(uint8_t addr, uint8_t dest,
                  uint8_t* key, bool encrypt,
                  uint8_t *buf, size_t buflen, size_t len) {
    // if there isn't enough room in the provided buffer for an hmac/hash
    if (buflen - len < 32) return -1;

    if (encrypt)
    message_digest(key, buf, len, buf+len);

    // pass buffer to message
    //message_init(addr);
    //message_send(dest, buf, len+ECDH_KEY_LENGTH);
    return 0;
}

int encrypt(uint8_t* key, uint8_t* in, size_t inlen, uint8_t* out) {
    uint8_t iv[16];
    int ret = 0;

    // Get 16 random bits for IV
    // TODO use mbedtls entropy
    rng_sync(iv, 16, 16);
    // place as prefix to encrypted content
    memcpy(out, iv, 16);

    mbedtls_aes_init(&aes_context);
    ret = mbedtls_aes_setkey_enc(&aes_context, key, ECDH_KEY_LENGTH*8);
    if(ret<0) return ret;

    ret = mbedtls_aes_crypt_cbc(&aes_context, MBEDTLS_AES_ENCRYPT, inlen, iv, in, out+16);
    if(ret<0) return ret;

    mbedtls_aes_free(&aes_context);
    return ret;
}

int decrypt(uint8_t* key, uint8_t iv[16], uint8_t* in, size_t inlen, uint8_t* out) {
    int ret = 0;

    mbedtls_aes_init(&aes_context);
    ret = mbedtls_aes_setkey_enc(&aes_context, key, ECDH_KEY_LENGTH*8);
    if(ret<0) return ret;

    ret = mbedtls_aes_crypt_cbc(&aes_context, MBEDTLS_AES_DECRYPT, inlen, iv, in, out);
    if(ret<0) return ret;

    mbedtls_aes_free(&aes_context);
    return ret;
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
