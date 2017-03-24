#include <stdio.h>
#include <stdlib.h>
#include <led.h>
#include <timer.h>
#include <sdcard.h>
#include <tock.h>

#include "fatfs/ff.h"

static FRESULT scan_files (const char* path) {
    FRESULT res;
    DIR dir;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       // Open the directory
    if (res == FR_OK) {
      while(1) {
        res = f_readdir(&dir, &fno);                   // Read a directory item
        if (res != FR_OK || fno.fname[0] == 0) break;  // Break on error or end of dir
        if (fno.fattrib & AM_DIR) {                    // It's a directory
          printf("+ %s/\n", fno.fname);
        } else {                                       // It's a file
          printf("  %s\n", fno.fname);
        }
      }
      f_closedir(&dir);
    }

    return res;
}


int main (void) {

  FATFS fs;           /* File system object */

  FRESULT res = f_mount(&fs, "", 1);

  while (res != FR_OK) {
    switch (res) {
      case FR_NOT_READY:
        printf("No disk found. Trying again...\n");
        delay_ms(5000);
        res = f_mount(&fs, "", 1);
        break;
      case FR_NO_FILESYSTEM:
        printf("No filesystem. Running mkfs...\n");
        {
          BYTE* work = malloc(_MAX_SS);
          f_mkfs("", FM_ANY, 0, work, _MAX_SS);
          free(work);
        }
        res = f_mount(&fs, "", 1);
        break;
      default:
        printf("Unexpected error! %d\n", res);
        return -1;
    }
  }

  printf("=== Mounted! Scanning root directory...\n");
  scan_files("");
  printf("=== Done.\n");

  while(1) {
    led_toggle(0);
    delay_ms(500);
  }
}

