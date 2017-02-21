#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_watchdog.h"
#include "console.h"
#include "i2c_master_slave.h"
#include "sdcard.h"
#include "signpost_api.h"
#include "signpost_energy.h"
#include "storage_master.h"
#include "timer.h"
#include "tock.h"
#include "gpio.h"

#include "signpost_storage.h"
#include "signbus_io_interface.h"

// buffer for holding i2c slave read data
#define SLAVE_READ_LEN 512
static uint8_t slave_read_buf[SLAVE_READ_LEN] = {0};

static void edison_wakeup(void) {
    gpio_clear(2);
    delay_ms(100);
    gpio_set(2);
}

static void processing_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, size_t message_length, uint8_t* message) {

    static bool rpc_pending = false;
    static uint8_t processing_src = 0x00;
    //static uint8_t processing_reason =  0x00;
    static uint16_t payload_len;
    static uint8_t* payload[1024];

    if(api_type != ProcessingApiType) {
        signpost_api_error_reply(source_address, api_type, message_type);
        return;
    }

    printf("got a processing callback\n");

    if(frame_type == NotificationFrame) {
        //shouldn't happen
    } else if (frame_type == CommandFrame) {
        printf("type %d\n", message_type);
        if(message_type == ProcessingInitMessage) {

            printf("Type Init\n");
            //save the message in the buffer to send to edison
            if(!rpc_pending) {
                payload_len = message_length;
                if(payload_len  > SLAVE_READ_LEN) {
                    //we can't send this message - respond with error?
                } else {
                    memcpy(payload,message,message_length);
                    processing_src = source_address;
                    //processing_reason = 0x00;
                    rpc_pending = true;
                }
            } else {
                //for now we are going to drop it - they can try
                //again in a bit
            }
            //wakeup edison
            edison_wakeup();

            //Edison will send a ProcessingEdisonReadReasonMessage
            //then we can send it the src addr and the function (init or rpc)
        } else if(message_type == ProcessingOneWayMessage) {
            //save the message in the buffer to send to edison
            printf("Type - One way message\n");
            if(!rpc_pending) {
                payload_len = message_length;
                if(payload_len  > SLAVE_READ_LEN) {
                    //we can't send this message - respond with error?
                } else {
                    memcpy(payload,message,message_length);
                    processing_src = source_address;
                    //processing_reason = 0x01;
                    rpc_pending = true;
                }
            } else {
                //for now we are going to drop it - they can try
                //again in a bit
            }
            //wakeup edison
            edison_wakeup();

            static uint8_t m = 0;
            signpost_processing_reply(processing_src,ProcessingOneWayMessage,&m,1);
            //Edison will send a ProcessingEdisonReadReasonMessage
            //then we can send it the src addr and the function (init or rpc)
        } else if(message_type == ProcessingTwoWayMessage) {
            //save the message in the buffer to send to edison
            printf("Type - Two way message\n");
            if(!rpc_pending) {
                payload_len = message_length;
                if(payload_len + 2 > SLAVE_READ_LEN) {
                    //we can't send this message - respond with error?
                } else {
                    memcpy(payload,message,message_length);
                    processing_src = source_address;
                    //processing_reason = 0x01;
                    rpc_pending = true;
                }
            } else {
                //for now we are going to drop it - they can try
                //again in a bit
            }
            //wakeup edison
            edison_wakeup();

            //just respond, we've done what we can
            //Edison will send a ProcessingEdisonReadReasonMessage
            //then we can send it the src addr and the function (init or rpc)
        } else if(message_type == ProcessingEdisonReadMessage) {
            //set up the read buffer with the proper stuff
            //and with the last function
            printf("Got edison read request");
            if(rpc_pending) {
                memcpy(slave_read_buf,payload,payload_len);
                //now a slave read should happen
            } else {
                //edison shouldn't be away
                //go back to sleep?
                //we can probably response with src addr 0 or something?
            }
        } else if(message_type == ProcessingEdisonResponseMessage) {
            printf("Got edison response");
            if(rpc_pending){
                signpost_processing_reply(processing_src,ProcessingEdisonResponseMessage,message,message_length);
                rpc_pending = false;
            } else {

            }
        }
    } else {
        //shouldn't happen
    }
}

static void storage_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, size_t message_length, uint8_t* message) {
  int err = SUCCESS;

  if (api_type != StorageApiType) {
    signpost_api_error_reply(source_address, api_type, message_type);
    return;
  }

  if (frame_type == NotificationFrame) {
    // XXX unexpected, drop
  } else if (frame_type == CommandFrame) {
    printf("Got a command message!: len = %d\n", message_length);
    for (uint32_t i=0; i<message_length; i++) {
      printf("%X ", message[i]);
    }
    printf("\n");

    //XXX do some checking that the message type is right and all that jazz
    //XXX also figure out what module index this is, somehow
    int module_index = 0;

    printf("Writing data\n");

    // get initial record
    Storage_Record_Pointer_t write_record = {0};
    write_record.block = storage_status.status_records[module_index].curr.block;
    write_record.offset = storage_status.status_records[module_index].curr.offset;

    // write data to storage
    err = storage_write_record(write_record, message, message_length, &write_record);
    if (err < SUCCESS) {
      printf("Writing error: %d\n", err);
      //XXX: send error
    }

    // update record
    storage_status.status_records[module_index].curr.block = write_record.block;
    storage_status.status_records[module_index].curr.offset = write_record.offset;
    err = storage_update_status();
    if (err < SUCCESS) {
      printf("Updating status error: %d\n", err);
      //XXX: send error
    }
    printf("Complete. Final block: %lu offset: %lu\n", write_record.block, write_record.offset);

    // send response
    err = signpost_storage_write_reply(source_address, (uint8_t*)&write_record);
    if (err < SUCCESS) {
      //XXX: I guess just try to send an error...
    }

  } else if (frame_type == ResponseFrame) {
    // XXX unexpected, drop
  } else if (frame_type == ErrorFrame) {
    // XXX unexpected, drop
  }
}

static void slave_read_callback (int err) {
  if (err < SUCCESS) {
    printf("I2C slave read error: %d\n", err);
  } else {
    // slave read complete, provide a new buffer
    printf("I2C slave read complete!\n");
    signbus_io_set_read_buffer(slave_read_buf, SLAVE_READ_LEN);
  }
}

int main (void) {

  int err = SUCCESS;
  printf("\n[Storage Master]\n** Storage API Test **\n");

    gpio_enable_output(2);
    gpio_set(2);

  // set up the SD card and storage system
  err = storage_initialize();
  if (err != SUCCESS) {
    printf(" - Storage initialization failed\n");
    return err;
  }

  // Install hooks for the signpost APIs we implement
  static api_handler_t storage_handler = {StorageApiType, storage_api_callback};
  static api_handler_t processing_handler = {ProcessingApiType, processing_api_callback};
  static api_handler_t* handlers[] = {&storage_handler, &processing_handler, NULL};
  signpost_initialization_module_init(ModuleAddressStorage, handlers);

  //XXX: TESTING
  for (int i=0; i<SLAVE_READ_LEN; i++) {
    slave_read_buf[i] = i;
  }

  // Setup I2C slave reads
  signbus_io_set_read_callback(slave_read_callback);
  signbus_io_set_read_buffer(slave_read_buf, SLAVE_READ_LEN);

  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("\nStorage Master initialization complete\n");
}

