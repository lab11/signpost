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

#include "signpost_storage.h"

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
    for (size_t i=0; i<message_length; i++) {
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

int main (void) {
  int err = SUCCESS;
  printf("\n[Storage Master]\n** Storage API Test **\n");

  // set up the SD card and storage system
  err = storage_initialize();
  if (err != SUCCESS) {
    printf(" - Storage initialization failed\n");
    return err;
  }

  // Install hooks for the signpost APIs we implement
  static api_handler_t storage_handler = {StorageApiType, storage_api_callback};
  static api_handler_t* handlers[] = {&storage_handler, NULL};
  signpost_initialization_storage_master_init(handlers);

  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("\nStorage Master initialization complete\n");
}

