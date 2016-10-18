#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "tock.h"
#include "console.h"
#include "isl29035.h"



int main () {
  putstr("[ISL29035] Test\n");

  int light = isl29035_read_light_intensity();
  {
    // Print the value
    char buf[64];
    sprintf(buf, "\tLight(%d) [0x%X]\n\n", light, light);
    putstr(buf);
  }
}
