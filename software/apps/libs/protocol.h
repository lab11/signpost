#pragma once

#include "module.h"
#include "app.h"

#define SHA256_LEN 32

/* protocol_send
 * Send a buffer through the protocol layer. Protocol layer calls message_send
 * to send buffer to dest.
 *   dest: i2c address of destination.
 *   key: buffer holding ECDH_KEY_LENGTH size key, if desired. If not NULL,
 *      protocol layer will encrypt with AES256-CTR and HMAC. If NULL, protocol
 *      layer will simply HASH contents.  buf: contents to send
 *   len: length of data to send
 *
 *   returns 0 on success, negative on failure.
 */
int protocol_send(uint8_t dest, uint8_t* key,
                  uint8_t *buf, size_t len);

/* protocol_recv
 * Receive buffer through the protocol layer.
 *   inbuf: raw receive buffer.
 *   inlen: length of receive data.
 *   key: buffer holding ECDH_KEY_LENGTH size key, if desired. If not NULL,
 *      protocol layer will check HMAC and decrypt with AES256-CTR. If NULL, protocol
 *      layer will simply check HASH. Must be the same key used to encrypt.
 *   appdata: buffer to hold decrypted/verified app data.
 *   applen: length of returned app data.
 *
 *   returns length of decrypted/authenticated buffer on success, negative on
 *   failure.
 */
int protocol_recv(uint8_t* buf, size_t buflen,
                  size_t len, uint8_t* key);

int protocol_recv_async(app_cb cb, uint8_t* buf, size_t buflen, uint8_t* key);
