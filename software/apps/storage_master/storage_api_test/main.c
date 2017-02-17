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

// should we wipe the SD card on boot or not?
// if true, we will read status off of the SD card and append to it
// if false, we will start from scratch, overwriting any old data
#define PERSISTENT_SDCARD 0

// default block size for SD card. Lots of things assume this
#define SDCARD_BLOCK_SIZE 512

// initial block to begin storing module data into
#define STORAGE_MODULES_INITIAL_BLOCK 2

// number of modules to allocate space for
#define STORAGE_MODULE_COUNT 8

// error for attempting to read from an invalid record
#define ENOHEADER -100

// storage status header
// stored in the beginning of the SD card
// keeps track of each module's storage locations
#define STORAGE_STATUS_BLOCK 0
#define STORAGE_STATUS_HEADER_OFFSET 0
#define STORAGE_STATUS_HEADER 0x11490811
typedef struct {
  uint32_t block;
  uint32_t offset;
} Storage_Record_Pointer_t;
typedef struct {
  Storage_Record_Pointer_t start;
  Storage_Record_Pointer_t curr;
} Storage_Status_Record_t;
typedef struct {
  uint32_t magic_header;
  uint32_t block_size;
  uint32_t blocks_per_module;
  Storage_Status_Record_t status_records[STORAGE_MODULE_COUNT];
} Storage_Status_t;

// individual record header
// keeps track of the size of this buffer of data
#define STORAGE_RECORD_HEADER 0x11
typedef struct __attribute__((__packed__)) {
  uint8_t magic_header;
  size_t message_length;
} Storage_Record_Header_t;

// State of current records
static Storage_Status_t storage_status = {0};

// buffers used by application
static uint8_t sdcard_buf[SDCARD_BLOCK_SIZE] = {0};

static uint32_t storage_write_data (Storage_Record_Pointer_t curr_record, uint8_t* buf, size_t len, Storage_Record_Pointer_t* new_record) {
  int err = SUCCESS;
  
  // need to write len bytes to SD card
  uint32_t bytes_to_write = len;

  // read in the first block
  uint32_t curr_block = curr_record.block;
  uint32_t curr_offset = curr_record.offset;
  err = sdcard_read_block_sync(curr_block);
  if (err < SUCCESS) {
    printf(" - SD card read error: %d\n", err);
    return err;
  }

  // write data split over multiple blocks as necessary
  uint32_t buf_offset = 0;
  while (bytes_to_write > 0) {

    // write whatever data fits to block
    uint32_t bytes_in_block = SDCARD_BLOCK_SIZE - curr_offset;
    if (bytes_to_write >= bytes_in_block) {
      // write as much as fits
      memcpy(&sdcard_buf[curr_offset], &buf[buf_offset], bytes_in_block);
      err = sdcard_write_block_sync(curr_block);
      if (err < SUCCESS) {
        printf(" - SD card write error: %d\n", err);
        return err;
      }

      // adjust offsets
      curr_offset = 0;
      curr_block++;
      buf_offset += bytes_in_block;
      bytes_to_write -= bytes_in_block;

    } else {
      // write the rest of the buffer
      memcpy(&sdcard_buf[curr_offset], &buf[buf_offset], bytes_to_write);
      err = sdcard_write_block_sync(curr_block);
      if (err < SUCCESS) {
        printf(" - SD card write error: %d\n", err);
        return err;
      }

      // update new record to this block and the next offset
      curr_offset += bytes_to_write;
      bytes_to_write = 0;
    }
  }

  // data written to SD card
  // Update new record
  if (new_record != NULL) {
    new_record->block = curr_block;
    new_record->offset = curr_offset;
  }
  return SUCCESS;
}

static uint32_t storage_read_data (Storage_Record_Pointer_t curr_record, uint8_t* buf, size_t len, Storage_Record_Pointer_t* next_record) {
  int err = SUCCESS;

  // need to read in len bytes from SD card
  uint32_t bytes_to_read = len;

  // set initial offsets
  uint32_t curr_block = curr_record.block;
  uint32_t curr_offset = curr_record.offset;

  // read data split over multiple blocks as necessary
  uint32_t buf_offset = 0;
  while (bytes_to_read > 0) {

    // read block
    err = sdcard_read_block_sync(curr_block);
    if (err < SUCCESS) {
      printf(" - SD card read error: %d\n", err);
      return err;
    }

    // read whatever data you can from the block
    uint32_t bytes_in_block = SDCARD_BLOCK_SIZE - curr_offset;
    if (bytes_to_read >= bytes_in_block) {
      // read as much as we can
      memcpy(&buf[buf_offset], &sdcard_buf[curr_offset], bytes_in_block);

      // adjust offsets
      curr_offset = 0;
      curr_block++;
      buf_offset += bytes_in_block;
      bytes_to_read -= bytes_in_block;
    } else {
      // read the rest of the record
      memcpy(&buf[buf_offset], &sdcard_buf[curr_offset], bytes_to_read);

      // adjust offsets
      curr_offset += bytes_to_read;
      bytes_to_read = 0;
    }
  }

  if (next_record != NULL) {
    next_record->block = curr_block;
    next_record->offset = curr_offset;
  }
  return SUCCESS;
}

static uint32_t storage_update_status (void) {
  Storage_Record_Pointer_t status_record_pointer = {
    .block = STORAGE_STATUS_BLOCK,
    .offset = STORAGE_STATUS_HEADER_OFFSET,
  };
  return storage_write_data(status_record_pointer, (uint8_t*)&storage_status, sizeof(Storage_Status_t), NULL);
}

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
    int module_index = 0;

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

static int storage_initialize (void) {
  int err = SUCCESS;
  printf("\nSetting up SD card\n");

  // check for SD card
  err = sdcard_is_installed();
  if (err < SUCCESS) {
    printf(" - sdcard_is_installed error: %d\n", err);
    return err;
  }
  if (err == 0) {
    printf(" - No SD card installed\n");
    return err;
  }
  printf(" * Found SD card. Initializing...\n");

  // initialize SD card
  uint32_t block_size = 0;
  uint32_t size_in_kB = 0;
  err = sdcard_initialize_sync(&block_size, &size_in_kB);
  if (err < SUCCESS) {
    printf(" - SD card initialization error: %d\n", err);
    return err;
  }
  if (block_size != SDCARD_BLOCK_SIZE) {
    // if the block size isn't 512 everything falls apart
    printf(" - SD card has invalid block size %lu\n", block_size);
    return FAIL;
  }
  printf(" * SD Card initialized.\tBlock size: %lu bytes\tSize: %lu kB\n",
      block_size, size_in_kB);

  // give buffers to SD card driver
  err = sdcard_set_read_buffer(sdcard_buf, SDCARD_BLOCK_SIZE);
  if (err < SUCCESS) {
    printf(" - SD card allow read error: %d\n", err);
    return err;
  }
  err = sdcard_set_write_buffer(sdcard_buf, SDCARD_BLOCK_SIZE);
  if (err < SUCCESS) {
    printf(" - SD card allow write error: %d\n", err);
    return err;
  }

  // read first block of the SD card
  err = sdcard_read_block_sync(STORAGE_STATUS_BLOCK);
  if (err < SUCCESS) {
    printf(" - SD card read error: %d\n", err);
    return err;
  }

  // determine if we already wrote a status structure
  // Only keep structure if PERSISTENT_SDCARD is true
  uint32_t header;
  memcpy(&header, &sdcard_buf[STORAGE_STATUS_HEADER_OFFSET], sizeof(header));
  if (PERSISTENT_SDCARD && (header == STORAGE_STATUS_HEADER)) {
    printf(" * Found Storage records on SD card\n");
    // status already exists, update our records to match
    memcpy(&storage_status, &sdcard_buf[STORAGE_STATUS_HEADER_OFFSET], sizeof(Storage_Status_t));

  } else {
    printf(" * Empty SD card. Initializing\n");
    // status does not exist. Create one
    storage_status.magic_header = STORAGE_STATUS_HEADER;
    storage_status.block_size = block_size;

    // figure out how to divide blocks between apps
    // divide between number of modules, leaving 1 KB at the start
    uint32_t blocks_per_module = ((size_in_kB-1) * 2) / STORAGE_MODULE_COUNT;
    storage_status.blocks_per_module = blocks_per_module;

    // initialize status record
    uint32_t initial_block = STORAGE_MODULES_INITIAL_BLOCK;
    for (int i=0; i<STORAGE_MODULE_COUNT; i++) {
      storage_status.status_records[i].start.block = initial_block + i*blocks_per_module;
      storage_status.status_records[i].start.offset = 0;
      storage_status.status_records[i].curr.block = initial_block + i*blocks_per_module;
      storage_status.status_records[i].curr.offset = 0;
    }

    // copy record to SD card
    err = storage_update_status();
    if (err < SUCCESS) {
      printf("Write error: %d\n", err);
      return err;
    }
  }

  // sdcard setup complete
  printf(" * SD card setup complete\n");
  return SUCCESS;
}

/*
//XXX TESTING
static void test_write_data (uint32_t module_index, uint8_t* message, size_t message_length) {
    // get initial record pointer
    Storage_Status_Record_t* curr_record = &storage_status.status_records[module_index];
    Storage_Record_Pointer_t orig_record_pointer = {
      .block = curr_record->curr.block,
      .offset = curr_record->curr.offset,
    };
    printf("Writing to Block %lu at Offset %lu\n", curr_record->curr.block, curr_record->curr.offset);

    // store data to SD card and get updated record pointer
    int err = SUCCESS;
    Storage_Record_Pointer_t new_record_pointer;

    // write the header
    Storage_Record_Header_t header = {
      .magic_header = STORAGE_RECORD_HEADER,
      .message_length = message_length,
    };
    printf("Size(%d)\n", sizeof(Storage_Record_Header_t));
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
}
*/

static uint32_t storage_write_record (Storage_Record_Pointer_t record, uint8_t* buf, size_t buf_len, Storage_Record_Pointer_t* next_record) {
  int err = SUCCESS;

  // write the header first
  Storage_Record_Pointer_t new_record = {0};
  Storage_Record_Header_t header = {
    .magic_header = STORAGE_RECORD_HEADER,
    .message_length = buf_len,
  };
  err = storage_write_data(record, (uint8_t*)&header, sizeof(Storage_Record_Header_t), &new_record);
  if (err < SUCCESS) {
    return err;
  }

  // write the message data
  err = storage_write_data(new_record, buf, buf_len, &new_record);
  if (err < SUCCESS) {
    return err;
  }

  // write complete, copy over values to user
  if (next_record != NULL) {
    next_record->block = new_record.block;
    next_record->offset = new_record.offset;
  }
  return SUCCESS;
}

static uint32_t storage_read_record (Storage_Record_Pointer_t record, uint8_t* buf, size_t* buf_len, Storage_Record_Pointer_t* next_record) {
  int err = SUCCESS;

  // read the record header first
  Storage_Record_Pointer_t new_record = {0};
  Storage_Record_Header_t header = {0};
  err = storage_read_data(record, (uint8_t*)&header, sizeof(Storage_Record_Header_t), &new_record);
  if (err < SUCCESS) {
    return err;
  }

  // check for header magic byte
  if (header.magic_header == STORAGE_RECORD_HEADER) {
    // header is valid, read buffer
    err = storage_read_data(new_record, buf, header.message_length, &new_record);
    if (err < SUCCESS) {
      return err;
    }
  } else {
    // bad header
    return ENOHEADER;
  }

  // read complete, copy over values to user
  if (buf_len != NULL) {
    *buf_len = header.message_length;
  }
  if (next_record != NULL) {
    next_record->block = new_record.block;
    next_record->offset = new_record.offset;
  }
  return SUCCESS;
}

//XXX: TESTING
static void storage_print_block (uint32_t block) {
  sdcard_read_block_sync(block);
  for (int i=0; i<SDCARD_BLOCK_SIZE; i+=16) {
    printf("%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X\n",
        sdcard_buf[i+0],
        sdcard_buf[i+1],
        sdcard_buf[i+2],
        sdcard_buf[i+3],
        sdcard_buf[i+4],
        sdcard_buf[i+5],
        sdcard_buf[i+6],
        sdcard_buf[i+7],
        sdcard_buf[i+8],
        sdcard_buf[i+9],
        sdcard_buf[i+10],
        sdcard_buf[i+11],
        sdcard_buf[i+12],
        sdcard_buf[i+13],
        sdcard_buf[i+14],
        sdcard_buf[i+15]);
  }
  printf("\n");
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
  signpost_initialization_module_init(ModuleAddressStorage, handlers);

  /*
  //XXX: TESTING
  storage_print_block(2);
  uint8_t array[10] = {0};
  for (int i=0; i<10; i++) {
    memset(array, i, 10);
    test_write_data(0, array, i+1);
    printf("i: %d\n", i);
    storage_print_block(2);
  }
  */

  //XXX: TESTING
  /*
  storage_print_block(2);
  uint8_t buffer[100] = {0};
  Storage_Record_Pointer_t record = {
    .block = 2,
    .offset = 0,
  };
  Storage_Record_Header_t header = {0};
  for (int i=0; i<10; i++) {
    printf("Header Block: %lu Offset: %lu\n", record.block, record.offset);
    storage_read_data(record, (uint8_t*)&header, sizeof(Storage_Record_Header_t), &record);
    printf("Header: %X, Length: %d\n", header.magic_header, header.message_length);

    if (header.magic_header == STORAGE_RECORD_HEADER) {
      printf("Data Block: %lu Offset: %lu\n", record.block, record.offset);
      storage_read_data(record, buffer, header.message_length, &record);
      for (int j=0; j<header.message_length; j++) {
        printf("%02X ", buffer[j]);
      }
      printf("\n");
    } else {
      printf("Bad header\n");
      break;
    }
  }
  */

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

