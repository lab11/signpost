#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <timer.h>
#include <tock.h>

#include "app_watchdog.h"
#include "signpost_api.h"
#include "simple_post.h"


static void networking_api_callback(uint8_t source_address,
                                    __attribute__ ((unused)) signbus_frame_type_t frame_type, 
                                    __attribute__ ((unused)) signbus_api_type_t api_type,
                                    __attribute__ ((unused)) uint8_t message_type, 
                                    size_t message_length, 
                                    uint8_t* message) {
    
    printf("I just received message: %.*s from module %02X\n",
              message_length, (char*)message, source_address);
}


int main (void) {
    printf("[simple_networking_test] ** Main App **\n");

    int rc;

    /////////////////////////////
    // Signpost Module Operations
    //
    // Initializations for the rest of the signpost
 
    //initialize the networking api for random module 
    static api_handler_t networking_handler = {NetworkingApiType, networking_api_callback};
    static api_handler_t* handlers[] = {&networking_handler, NULL}; 
   
    //perform module initialization for address 0x30 (because that's what send test uses)
    do {
      rc = signpost_initialization_module_init(0x30, handlers);
      if (rc < 0) {
        printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
        delay_ms(5000);
      }
    } while (rc < 0);

    
    //just yield and let the networking API do the work

}
