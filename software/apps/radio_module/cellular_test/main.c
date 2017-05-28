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
    printf("Posting...\n");
    int ret = sara_u260_basic_http_post("httpbin.org","/post",buf,5);

    if(ret == SARA_U260_NO_SERVICE) {
        printf("No Service! try again in a bit.\n");
    }

    delay_ms(10000);

    if(ret >= 0) {
        uint8_t dbuf[500] = {0};
        printf("Getting response\n");
        int l = sara_u260_get_post_response(dbuf, 499);
        if(l < 0) {
            printf("Did not get response\n");
        } else {
            dbuf[l] = 0;
            printf("Got resposne of length %d: \n",l);
            printf("%s\n", (char*)dbuf);
        }
    }
}
