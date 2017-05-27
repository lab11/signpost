#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "tock.h"
#include "timer.h"
#include "console.h"
#include "led.h"
#include "sara_u260.h"


int main (void) {
    printf("Starting Cell Test\n");

    delay_ms(2000);

    sara_u260_init();

    delay_ms(1000);

    uint8_t buf[5] = {'L','a','b','1','1'};
    sara_u260_basic_http_post("httpbin.org","/post",buf,5);
}
