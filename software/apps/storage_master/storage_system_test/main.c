#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <i2c_master_slave.h>
#include <sdcard.h>
#include <timer.h>
#include <tock.h>

#include "app_watchdog.h"
#include "signpost_api.h"
#include "signpost_energy.h"
#include "signpost_storage.h"
#include "storage_master.h"

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
    /*
    int module_index = 0;
    */

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

    /*
       if (message_type == EnergyQueryMessage) {
       signpost_energy_information_t info;
       info.energy_limit_24h_mJ = 1;
       info.energy_used_24h_mJ = 2;
       info.current_limit_60s_mA = 3;
       info.current_average_60s_mA = 4;
       info.energy_limit_warning_threshold = 5;
       info.energy_limit_critical_threshold = 6;

       signpost_energy_query_reply(source_address, &info);
       } else if (message_type == EnergyLevelWarning24hMessage) {
       signpost_api_error_reply(source_address, api_type, message_type);
       } else if (message_type == EnergyLevelCritical24hMessage) {
       signpost_api_error_reply(source_address, api_type, message_type);
       } else if (message_type == EnergyCurrentWarning60sMessage) {
       signpost_api_error_reply(source_address, api_type, message_type);
       } else {
       signpost_api_error_reply(source_address, api_type, message_type);
       }
       */
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
    return err;
  }

  // Install hooks for the signpost APIs we implement
  static api_handler_t storage_handler = {StorageApiType, storage_api_callback};
  static api_handler_t* handlers[] = {&storage_handler, NULL};
  signpost_initialization_storage_master_init(handlers);


  //XXX: TESTING
  // Write data to SD card
  while (1) {
    printf("Writing data\n");
    uint8_t module_index = 0;
    Storage_Record_Pointer_t write_record = {0};
    write_record.block = storage_status.status_records[module_index].curr.block;
    write_record.offset = storage_status.status_records[module_index].curr.offset;
    uint8_t buffer[100] = {0};
    for (int i=0; i<100; i++) {
      memset(buffer, i, i);
      err = storage_write_record(write_record, buffer, i, &write_record);
      if (err < SUCCESS) {
        printf("Writing error: %d\n", err);
        break;
      }
    }
    storage_status.status_records[module_index].curr.block = write_record.block;
    storage_status.status_records[module_index].curr.offset = write_record.offset;
    err = storage_update_status();
    if (err < SUCCESS) {
      printf("Updating status error: %d\n", err);
    }
    printf("Complete. Final block: %lu offset: %lu\n", write_record.block, write_record.offset);

    // Read data from SD card
    printf("Reading data\n");
    Storage_Record_Pointer_t read_record = {
      .block = 2,
      .offset = 0,
    };
    err = SUCCESS;
    size_t len = 0;
    while (err == SUCCESS) {
      err = storage_read_record(read_record, buffer, &len, &read_record);
      if (err == SUCCESS) {
        /*
        for (int j=0; j<len; j++) {
          printf("%02X ", buffer[j]);
        }
        printf("\n");
        */
      } else if (err == ENOHEADER) {
        // no more records to read
        break;
      } else {
        printf("Reading error: %d\n", err);
        break;
      }
    }
    printf("Complete\n");

    printf("\n");
  }


  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("\nStorage Master initialization complete\n");
}

