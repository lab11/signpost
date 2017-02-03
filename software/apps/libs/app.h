#include <stdarg.h>

typedef struct {
    uint8_t len;
    uint8_t arg[256];
} Arg;

int app_send(uint8_t addr, uint8_t dest, uint8_t* key,
             uint8_t cmdrsp, uint8_t type,
             uint8_t func, int numarg, Arg* args);


int app_recv(uint8_t* key, uint8_t* cmdrsp, uint8_t* type,
             uint8_t* func, Arg* args, size_t* numarg);
