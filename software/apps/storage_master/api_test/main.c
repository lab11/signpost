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

  if (api_type != StorageApiType) {
    signpost_api_error_reply(source_address, api_type, message_type);
    return;
  }

  if (frame_type == NotificationFrame) {
    // XXX unexpected, drop
  } else if (frame_type == CommandFrame) {
    printf("Got a command message!\n");
    //XXX write data to SD card and update usage block
    //  first word written ought to be length each time
    //  then the record_pointer will point to that length
    //  and we can know how much to read out

    // this will have to be a loop where we read in blocks,
    // append data
    // write block to sd card
    // until all data is written

    //XXX do some checking that the message type is right and all that jazz
    //XXX also figure out what module index this is, somehow
    int module_index = 0;

    //XXX: rewrite this using storage_write_record()
    /*
    // get initial record pointer
    Storage_Status_Record_t* curr_record = &storage_status.status_records[module_index];
    Storage_Record_Pointer_t orig_record_pointer = {
      .block = curr_record->curr.block,
      .offset = curr_record->curr.offset,
    };

    // store data to SD card and get updated record pointer
    int err = SUCCESS;
    Storage_Record_Pointer_t new_record_pointer;

    // write the header
    Storage_Record_Header_t header = {
      .magic_header = STORAGE_RECORD_HEADER,
      .message_length = message_length,
    };
    err = storage_write_data(orig_record_pointer, (uint8_t*)&header, sizeof(Storage_Record_Header_t), &new_record_pointer);
    if (err < SUCCESS) {
      //XXX: respond with error
    }

    // write the message data
    err = storage_write_data(new_record_pointer, message, message_length, &new_record_pointer);
    if (err < SUCCESS) {
      //XXX: respond with error
    }

    // update record and store to SD card
    curr_record->curr.block = new_record_pointer.block;
    curr_record->curr.offset = new_record_pointer.offset;
    err = storage_update_status();
    if (err < SUCCESS) {
      //XXX: respond with error
    }
    */
    

    //XXX send complete response with orig_record_pointer to app

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
  signpost_initialization_module_init(ModuleAddressStorage, handlers);

  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("\nStorage Master initialization complete\n");
}

