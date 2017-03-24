Storage Module FAT Filesystem Test
=========================

This is a test application to prototype interacting with a FAT32-formatted SD
Card. It uses the the FatFS library from
<http://elm-chan.org/fsw/ff/00index_e.html>, vendored in the `fat/`
subdirectory.

The test program in `main.c` waits for an SD Card to be available. Then, it
tries to mount it as a FAT filesystem. If no FAT filesystem is present, it
formats the card first. Finally, the test program lists all files in the root
directory.

## Compatibility layer

The FatFS library requires implementing a HAL for interacting with the
underlying storage device. In this case, it's implemented in `mmc_io.c` on top
of the SD Card library (which uses an SD Card driver in the kernel). While the
test program itself is fairly rudadentary, this implementation along with the
library should be sufficient to build a "real" application.
