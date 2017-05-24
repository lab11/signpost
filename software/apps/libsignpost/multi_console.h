#pragma once

#include <stdint.h>
#include <tock.h>

size_t console_read(int console_num, uint8_t* buf, size_t max_len);
void   console_read_async(int console_num, uint8_t* buf, size_t max_len, subscribe_cb cb);
size_t console_write(int console_num, uint8_t* buf, size_t count);
void   console_write_async(int console_num, uint8_t* buf, size_t count, subscribe_cb cb);
