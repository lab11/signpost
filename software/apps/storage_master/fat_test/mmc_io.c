#include <sdcard.h>
#include "fatfs/diskio.h"
#include "fatfs/ff.h"

static uint32_t block_size;
static uint32_t card_kb;
static DSTATUS status = STA_NOINIT;


DSTATUS disk_initialize (BYTE pdrv) {
  if (pdrv != 0) {
    return STA_NODISK;
  }

  int err = sdcard_initialize_sync(&block_size, &card_kb);

  if (err != 0) {
    status |= STA_NODISK;
  } else {
    status &= ~STA_NOINIT;
  }
  return status;
}

DSTATUS disk_status (BYTE pdrv) {
  if (pdrv != 0) {
    return STA_NODISK;
  }

  return status;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
  if (pdrv != 0) {
    return RES_PARERR;
  }

  if ((status & STA_NOINIT) != 0) {
    return RES_NOTRDY;
  }

  for (UINT i = 0; i < count; i++) {
    sdcard_set_read_buffer(buff + (_MAX_SS * i), _MAX_SS);
    sdcard_read_block_sync(sector + i);
  }
  return RES_OK;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
  if (pdrv != 0) {
    return RES_PARERR;
  }

  if ((status & STA_NOINIT) != 0) {
    return RES_NOTRDY;
  }

  for (UINT i = 0; i < count; i++) {
    sdcard_set_write_buffer((BYTE*)(buff + (_MAX_SS * i)), _MAX_SS);
    sdcard_write_block_sync(sector + i);
  }
  return RES_OK;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
  if (pdrv != 0) {
    return RES_PARERR;
  }

  if ((status & STA_NOINIT) != 0) {
    return RES_NOTRDY;
  }

  switch (cmd) {
    case CTRL_SYNC:
      break;
    case GET_SECTOR_COUNT:
      *(DWORD*)buff = card_kb / block_size * 1024;
      break;
    case GET_SECTOR_SIZE:
      *(WORD*)buff = block_size;
      break;
    case GET_BLOCK_SIZE:
      *(DWORD*)buff = 1;
      break;
    case CTRL_TRIM:
      break;
    default:
      return RES_PARERR;
  }

  return RES_OK;
}

