#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_FM25CL 103

int fm25cl_set_callback (subscribe_cb callback, void* callback_args);
int fm25cl_set_read_buffer(uint8_t* buffer, uint32_t len);
int fm25cl_set_write_buffer(uint8_t* buffer, uint32_t len);
int fm25cl_read_status(void);
int fm25cl_read(uint16_t address, uint16_t len);
int fm25cl_write(uint16_t address, uint16_t len);

int fm25cl_read_status_sync(void);
int fm25cl_read_sync(uint16_t address, uint16_t len);
int fm25cl_write_sync(uint16_t address, uint16_t len);

#ifdef __cplusplus
}
#endif
