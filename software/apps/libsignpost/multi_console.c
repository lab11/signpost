#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <tock.h>
#include "multi_console.h"

typedef struct {
    int len;
    bool fired;
} console_callback_struct;


static void console_callback (
        __attribute__ ((unused)) int len,
        __attribute__ ((unused)) int x,
        __attribute__ ((unused)) int y,
        void* userdata) {

   console_callback_struct* cp = (console_callback_struct*)userdata;
   cp->fired = true;
   cp->len = len;
}

size_t console_read(int console_num, uint8_t* buf, size_t max_len) {
    allow(console_num, 0, (void*)buf, max_len);
    console_callback_struct c;
    c.fired = false;
    subscribe(console_num, 2, console_callback, &c);

    yield_for(&c.fired);

    return c.len;
}

void   console_read_async(int console_num, uint8_t* buf, size_t max_len, subscribe_cb cb) {
    allow(console_num, 0, (void*)buf, max_len);
    subscribe(console_num, 2, cb, NULL);
}

size_t console_write(int console_num, uint8_t* buf, size_t count) {

    uint8_t* cbuf = (uint8_t*)malloc(count * sizeof(uint8_t));
    memcpy(cbuf, buf, count);

    allow(console_num, 1, (void*)cbuf, count);
    console_callback_struct c;
    c.fired = false;
    subscribe(console_num, 1, console_callback, &c);

    yield_for(&c.fired);

    return c.len;
}

void console_write_async(int console_num, uint8_t* buf, size_t count, subscribe_cb cb) {
    allow(console_num, 1, (void*)buf, count);
    subscribe(console_num, 1, cb, NULL);
}
