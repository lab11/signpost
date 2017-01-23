#include <stdarg.h>

int app_send(uint8_t addr, uint8_t dest, uint8_t* key,
             uint8_t cmdrsp, uint8_t type,
             uint8_t func, int numarg, ...);
