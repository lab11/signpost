#pragma once

// error for attempting to read from an invalid record
#define ENOHEADER -100

// storage record pointer
// The means of pointing to a data item on the SD card
typedef struct {
  uint32_t block;
  uint32_t offset;
} Storage_Record_Pointer_t;

// number of modules to allocate space for
#define STORAGE_MODULE_COUNT 8

// storage status header
// used to keep track of module logs
// written to start of the SD card
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

// storage record header
// used at the head of each record written to the SD card
typedef struct __attribute__((__packed__)) {
  uint8_t magic_header;
  size_t message_length;
} Storage_Record_Header_t;

// State of current records
// defined in storage.c
extern Storage_Status_t storage_status;

// function prototypes

// writes storage_status to the SD card
// The user should have already updated storage_status
int32_t storage_update_status (void);

// writes a record to the SD card and returns a pointer for the next record
// Expects the user to update storage_status
int32_t storage_write_record (Storage_Record_Pointer_t record, uint8_t* buf, size_t buf_len, Storage_Record_Pointer_t* next_record);

// reads a record from the SD card and returns a pointer for the next record
// Returns ENOHEADER if the pointer supply is not to a valid block
int32_t storage_read_record (Storage_Record_Pointer_t record, uint8_t* buf, size_t* buf_len, Storage_Record_Pointer_t* next_record);

// initializes the SD card and storage system on top of it
int32_t storage_initialize (void);

