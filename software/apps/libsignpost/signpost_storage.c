// storage interface to layer on top of the SD card

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sdcard.h"
#include "tock.h"

#include "signpost_storage.h"

// should we wipe the SD card on boot or not?
// if true, we will read status off of the SD card and append to it
// if false, we will start from scratch, overwriting any old data
#define PERSISTENT_SDCARD 1

// default block size for SD card. Lots of things assume this
#define SDCARD_BLOCK_SIZE 512

// initial block to begin storing module data into
#define STORAGE_MODULES_INITIAL_BLOCK 2

// storage status header
// stored in the beginning of the SD card
// keeps track of each module's storage locations
#define STORAGE_STATUS_BLOCK 0
#define STORAGE_STATUS_HEADER_OFFSET 0
#define STORAGE_STATUS_HEADER 0x11490811

// individual record header
// keeps track of the size of this buffer of data
#define STORAGE_RECORD_HEADER 0x11

// State of current records
Storage_Status_t storage_status = {0};

// buffers used by application
static uint8_t sdcard_buf[SDCARD_BLOCK_SIZE] = {0};

//XXX: TESTING
// static void storage_print_block (uint32_t block) {
//   sdcard_read_block_sync(block);
//   for (int i=0; i<SDCARD_BLOCK_SIZE; i+=16) {
//     printf("%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X\n",
//         sdcard_buf[i+0],
//         sdcard_buf[i+1],
//         sdcard_buf[i+2],
//         sdcard_buf[i+3],
//         sdcard_buf[i+4],
//         sdcard_buf[i+5],
//         sdcard_buf[i+6],
//         sdcard_buf[i+7],
//         sdcard_buf[i+8],
//         sdcard_buf[i+9],
//         sdcard_buf[i+10],
//         sdcard_buf[i+11],
//         sdcard_buf[i+12],
//         sdcard_buf[i+13],
//         sdcard_buf[i+14],
//         sdcard_buf[i+15]);
//   }
//   printf("\n");
// }

static int32_t storage_write_data (Storage_Record_Pointer_t curr_record, uint8_t* buf, size_t len, Storage_Record_Pointer_t* new_record) {
  int32_t err = SUCCESS;

  // need to write len bytes to SD card
  uint32_t bytes_to_write = len;

  // read in the first block
  uint32_t curr_block = curr_record.block;
  uint32_t curr_offset = curr_record.offset;
  err = sdcard_read_block_sync(curr_block);
  if (err < SUCCESS) {
    printf(" - SD card read error: %ld\n", err);
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
        printf(" - SD card write error: %ld\n", err);
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
        printf(" - SD card write error: %ld\n", err);
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

static int32_t storage_read_data (Storage_Record_Pointer_t curr_record, uint8_t* buf, size_t len, Storage_Record_Pointer_t* next_record) {
  int32_t err = SUCCESS;

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
      printf(" - SD card read error: %ld\n", err);
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

int32_t storage_update_status (void) {
  Storage_Record_Pointer_t status_record_pointer = {
    .block = STORAGE_STATUS_BLOCK,
    .offset = STORAGE_STATUS_HEADER_OFFSET,
  };
  return storage_write_data(status_record_pointer, (uint8_t*)&storage_status, sizeof(Storage_Status_t), NULL);
}

int32_t storage_write_record (Storage_Record_Pointer_t record, uint8_t* buf, size_t buf_len, Storage_Record_Pointer_t* next_record) {
  int32_t err = SUCCESS;

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

int32_t storage_read_record (Storage_Record_Pointer_t record, uint8_t* buf, size_t* buf_len, Storage_Record_Pointer_t* next_record) {
  int32_t err = SUCCESS;

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

int32_t storage_initialize (void) {
  int32_t err = SUCCESS;
  printf("\nSetting up SD card\n");

  // check for SD card
  err = sdcard_is_installed();
  if (err < SUCCESS) {
    printf(" - sdcard_is_installed error: %ld\n", err);
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
    printf(" - SD card initialization error: %ld\n", err);
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
    printf(" - SD card allow read error: %ld\n", err);
    return err;
  }
  err = sdcard_set_write_buffer(sdcard_buf, SDCARD_BLOCK_SIZE);
  if (err < SUCCESS) {
    printf(" - SD card allow write error: %ld\n", err);
    return err;
  }

  // read first block of the SD card
  err = sdcard_read_block_sync(STORAGE_STATUS_BLOCK);
  if (err < SUCCESS) {
    printf(" - SD card read error: %ld\n", err);
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
      printf("Write error: %ld\n", err);
      return err;
    }
  }

  // sdcard setup complete
  printf(" * SD card setup complete\n");
  return SUCCESS;
}

