#pragma once

#include "signbus_app_layer.h"
#include "signbus_protocol_layer.h"

#define SHA256_LEN 32
#define ECDH_KEY_LENGTH 32

/// Send a buffer through the protocol layer.
/// The protocol layer will encrypt the payload using the provided
/// ECDH_KEY_LENGTH key with AES256-CTR and HMAC. If no key is provided,
/// the payload with simply be HASHed.
///
/// Returns number of bytes sent, or < 0 on failure
int signbus_protocol_send(
    uint8_t dest,                     // Address to send to
    uint8_t* key,                     // Key to encrypt with or NULL for clear
    uint8_t* buf,                     // Buffer to send from
    size_t len                        // Number of bytes to send
    );

/// Receive buffer through the protocol layer.
///  key: buffer holding ECDH_KEY_LENGTH size key, if desired. If not NULL,
///     protocol layer will check HMAC and decrypt with AES256-CTR. If NULL, protocol
///     layer will simply check HASH. Must be the same key used to encrypt.
/// Returns length of decrypted/authenticated buffer on success, < 0 on error.
int signbus_protocol_recv(
    uint8_t *sender_address,          // I2C address of sender
    uint8_t* (*addr_to_key)(uint8_t), // Translation function from address-> key
    size_t recv_buflen,               // Buffer length
    uint8_t* recv_buf                 // Buffer to recieve into
    );

/// Set up async buffer.
/// For synchronous receives, all necessary buffers will be allocated on the
/// call stack. Asynchronous recieves require an additional buffer to store
/// the raw encrypted payload. This buffer should be a little bit larger
/// (minimally 16 + 16 + 32 + 4 = 68 bytes) than the largest message you
/// expected to recieve to account for protocol overhead.
///
/// XXX: I'm not sure I love this. I do like that it gets rid of the giant
///      buffers for apps that don't use them. I don't love that apps have
///      to call this magic function in the protocol layer to get things to
///      work. That's an interface failure right there. It's okay for now,
///      as the only things that will see this are the signpost_api demux
///      and the specialized test app, but something should be done.
void signbus_protocol_setup_async(
    uint8_t* buf,                     // Buffer to write to. NULL to disable
    size_t   buflen                   // Lenght of buf in bytes
    );

/// async callback
/// len_or_rc is number of bytes received of < 0 on error.
typedef void (*signbus_protocol_callback_t)(int len_or_rc);

int signbus_protocol_recv_async(
    signbus_protocol_callback_t cb,   // Called when recv operation completes
    uint8_t* sender_address,          // I2C address of sender
    uint8_t* (*addr_to_key)(uint8_t),          // Translation function from address -> key
    size_t recv_buflen,               // Buffer length
    uint8_t* recv_buf                 // Buffer to recieve into
    );
