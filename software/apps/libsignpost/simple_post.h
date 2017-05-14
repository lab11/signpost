#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int simple_octetstream_post(const char* url, uint8_t* buf, uint16_t buf_len);

#ifdef __cplusplus
}
#endif
