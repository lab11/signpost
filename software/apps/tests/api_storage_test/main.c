#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "signpost_api.h"
#include "timer.h"
#include "tock.h"

static const uint8_t i2c_address = 0x50;

#define DATA_SIZE 600
static uint8_t data[DATA_SIZE] = {0};

int main (void) {
  int err;
  putstr("[Test] API: Storage\n");

  signpost_initialization_module_init(
      i2c_address,
      SIGNPOST_INITIALIZATION_NO_APIS);

  Storage_Record_t record = {0};
  while (true) {
    // create buffer
    // set buffer value to a byte from the record so that the value changes
    memset(data, record.value[4], DATA_SIZE);
    printf("Writing buffer [%X]*%d\n", record.value[4], DATA_SIZE);

    err = signpost_storage_write(data, DATA_SIZE, &record);
    if (err < SUCCESS) {
      printf("Error writing to storage: %d\n", err);
    } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
      printf("Wrote successfully! Block: %lu Offset: %lu\n", *(uint32_t*)&record.value[0], *(uint32_t*)&record.value[4]);
#pragma GCC diagnostic pop
    }
    printf("\n");

    // sleep for a second
    delay_ms(1000);
  }
}

