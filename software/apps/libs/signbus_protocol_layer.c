#include <stdio.h>
#include <string.h>

#include "mbedtls/md.h"
#include "mbedtls/cipher.h"

#include "signpost_entropy.h"
#include "signbus_app_layer.h"
#include "signbus_io_interface.h"
#include "signbus_protocol_layer.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

/// Protocol Layer Operation
///
/// Encryption/Decryption routines require source and destination buffers,
/// so some copying is unavoidable. For both sending and receiving, we make
/// temporary buffer that matches or slightly exceeds the request size's
/// buffer. When sending, use the request buffer as cleartext and our buffer
/// for ciphertext, passing our buffer to the next layer. When receiving,
/// receive into our buffer, so that the decryption routine writes into the
/// final destination buffer.

static const mbedtls_md_info_t * md_info;
static mbedtls_md_context_t md_context;
static const mbedtls_cipher_info_t * cipher_info;
static mbedtls_cipher_context_t cipher_context;

static int cipher(
        const mbedtls_operation_t operation,
        uint8_t* key, uint8_t* iv,
        uint8_t* in, size_t inlen,
        uint8_t* out, size_t* olen
        ) {
    SIGNBUS_DEBUG("op 0x%x key %p iv %p in %p inlen %u out %p olen %p (%u)\n",
            operation, key, iv, in, inlen, out, olen, *olen);

    uint8_t ivenc[MBEDTLS_MAX_IV_LENGTH];
    int ret = 0;

    if (operation == MBEDTLS_ENCRYPT) {
        // Get 16 random bits for IV
        ret = signpost_entropy_rand(ivenc, MBEDTLS_MAX_IV_LENGTH);
        if (ret < 0) return ret;
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

static int message_digest(uint8_t* key, uint8_t* in, size_t inlen, uint8_t* out) {
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

int signbus_protocol_send(
        uint8_t dest,
        uint8_t* (*addr_to_key)(uint8_t),
        uint8_t* clear_buf,
        size_t clear_buflen
        ) {
    uint8_t* key = addr_to_key(dest);

    SIGNBUS_DEBUG("dest %02x key %p clear_buf %p clear_buflen %d\n",
            dest, key, clear_buf, clear_buflen);

    // Needs to be multiple of block size + 1 additional block
    const size_t encrypted_buflen = 16*(clear_buflen/16+(clear_buflen%16 != 0)) + 16;

    // Also allocate space for hash and IV
    const size_t protocol_buflen =
        MBEDTLS_MAX_IV_LENGTH + encrypted_buflen + SHA256_LEN;

    // sendbuf is what is sent to message layer, needs to fit hash and IV
    //uint8_t sendbuf[len+MBEDTLS_MAX_IV_LENGTH+SHA256_LEN];
    uint8_t protocol_buf[protocol_buflen];
    size_t protocol_buf_used = 0;

    if(key!=NULL) {
        uint8_t* iv = protocol_buf;
        protocol_buf_used += MBEDTLS_MAX_IV_LENGTH;

        // encrypt buf
        size_t encrypted_buf_used;
        uint8_t* encrypted_buf = protocol_buf + MBEDTLS_MAX_IV_LENGTH;
        int rc;
        rc = cipher(MBEDTLS_ENCRYPT, key, iv,
                clear_buf, clear_buflen,
                encrypted_buf, &encrypted_buf_used);
        if (rc < 0) return rc;

        protocol_buf_used += encrypted_buf_used;
    }
    // otherwise just hash over content
    else {
        memcpy(protocol_buf, clear_buf, clear_buflen);
        protocol_buf_used += clear_buflen;
    }

    // hmac over current protocol payload
    uint8_t* hmac = protocol_buf + protocol_buf_used;
    message_digest(key, protocol_buf, protocol_buf_used, hmac);
    protocol_buf_used += SHA256_LEN;

    // pass buffer to message
    // expects message_init to have been called by module_init
    return signbus_io_send(dest, protocol_buf, protocol_buf_used);
}


/// Decrypt a buffer
/// Returns number of cleartext payload bytes or < 0 if error.
static int protocol_encrypted_buffer_received(
        uint8_t* key,
        uint8_t* protocol_buf,
        size_t   protocol_buflen,
        uint8_t* output_buf,
        size_t   output_buflen
        ) {
    SIGNBUS_DEBUG("key %p proto buf %p len %u ouput buf %p len %u\n",
            key, protocol_buf, protocol_buflen, output_buf, output_buflen);
    SIGNBUS_DEBUG_DUMP_BUF(protocol_buf, protocol_buflen);

    // Basic sanity check
    size_t sane_length = SHA256_LEN + (key == NULL) ? 0 : MBEDTLS_MAX_IV_LENGTH;
    if (protocol_buflen < sane_length) {
        // TODO: Meaningful return codes. Let's at least try to be unique
        return -93;
    }

    // Check HMAC or hash
    uint8_t hmac_or_hash[SHA256_LEN];
    message_digest(key, protocol_buf, protocol_buflen-SHA256_LEN, hmac_or_hash);
    if (memcmp(hmac_or_hash, protocol_buf+(protocol_buflen-SHA256_LEN), SHA256_LEN) != 0) {
        // TODO: Meaningful return codes. Let's at least try to be unique
        return -94;
    }

    // decrypt if needed
    size_t clear_len;

    if(key != NULL) {
        _Static_assert(MBEDTLS_MAX_IV_LENGTH == 16, "iv len in proto match");
        // First 16 bytes in buffer for protocol layer are IV, then the payload

        uint8_t* iv = protocol_buf;
        uint8_t* encrypted_buf = protocol_buf + MBEDTLS_MAX_IV_LENGTH;
        const size_t encrypted_buflen =
            protocol_buflen - MBEDTLS_MAX_IV_LENGTH - SHA256_LEN;
        if (output_buflen < encrypted_buflen) {
            // This size restriction comes from mbed
            // TODO: Meaningful return codes. Let's at least try to be unique
            return -95;
        }

        printf("buffer before decrypt:\n0x");
        for(int i = 0; i < encrypted_buflen; i++) {
            printf("%x", encrypted_buf[i]);
        }
        printf("\n");
        int rc;
        rc = cipher(MBEDTLS_DECRYPT,
                key, iv,
                encrypted_buf, encrypted_buflen,
                output_buf, &clear_len);
        if (rc < 0) return rc;
        printf("buffer after decrypt:\n0x");
        for(int i = 0; i < clear_len; i++) {
            printf("%x", output_buf[i]);
        }
        printf("\n");
    } else {
        clear_len = protocol_buflen - SHA256_LEN;
        if (output_buflen < clear_len) {
            // TODO: Meaningful return codes. Let's at least try to be unique
            return -96;
        }
        memcpy(output_buf, protocol_buf, clear_len);
    }

    SIGNBUS_DEBUG_DUMP_BUF(output_buf, clear_len);
    return clear_len;
}


int signbus_protocol_recv(
        uint8_t* sender_address,
        uint8_t* (*addr_to_key)(uint8_t),
        size_t clear_buflen,
        uint8_t* clear_buf
        ) {
    // Assumption: The user does not pass a buffer smaller than the message
    // they plan to receive. We need a slightly bigger one to account for
    // encryption block size, protocol IV and HMAC overhead. The math is the
    // same as the send path.

    // Needs to be multiple of block size + 1 additional block
    const size_t encrypted_buflen = 16*(clear_buflen/16+(clear_buflen%16 != 0)) + 16;

    // Also allocate space for hash and IV
    const size_t protocol_buflen =
        MBEDTLS_MAX_IV_LENGTH + encrypted_buflen + SHA256_LEN;

    uint8_t protocol_buf[protocol_buflen];

    int len_or_rc = signbus_io_recv(protocol_buflen, protocol_buf, sender_address);
    if (len_or_rc < 0) return len_or_rc;

    uint8_t* key = (addr_to_key == NULL) ? NULL : addr_to_key(*sender_address);

    return protocol_encrypted_buffer_received(key,
            protocol_buf, len_or_rc,
            clear_buf, clear_buflen);
}

typedef struct {
    signbus_app_callback_t* cb;
    uint8_t* sender_address;
    uint8_t* (*addr_to_key)(uint8_t);
    size_t buflen;
    uint8_t* buf;
} prot_cb_data;

static prot_cb_data cb_data;

static uint8_t* async_buf = NULL;
static size_t async_buflen = 0;

void signbus_protocol_setup_async(uint8_t* buf, size_t buflen) {
    async_buf = buf;
    async_buflen = buflen;
}

static void protocol_async_callback(int len_or_rc) {
    if (len_or_rc < 0) return cb_data.cb(len_or_rc);

    uint8_t* key = (cb_data.addr_to_key == NULL) ? NULL : cb_data.addr_to_key(*cb_data.sender_address);

    len_or_rc = protocol_encrypted_buffer_received(key,
            async_buf, len_or_rc,
            cb_data.buf, cb_data.buflen);
    cb_data.cb(len_or_rc);
}

int signbus_protocol_recv_async(
        signbus_app_callback_t cb,
        uint8_t* sender_address,
        uint8_t* (*addr_to_key)(uint8_t),
        size_t recv_buflen,
        uint8_t* recv_buf
        ) {
    if (async_buf == NULL) {
        // Need to call signbus_protocol_setup_async first
        // TODO: Meaningful return codes. Let's at least try to be unique
        return -97;
    }
    cb_data.cb = cb;
    cb_data.addr_to_key = addr_to_key;
    cb_data.sender_address = sender_address;
    cb_data.buflen = recv_buflen;
    cb_data.buf = recv_buf;

    return signbus_io_recv_async(protocol_async_callback,
            async_buflen, async_buf, sender_address);
}
