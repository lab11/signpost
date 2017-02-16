### Clone a live Edision Image

As root:
Perform an emergency remount of the root filesystem
```
echo u > /proc/sysrq-trigger
```
Mount the sdcard
```
mkdir /media/sdcard
mount /dev/mmcblk1p1 /media/sdcard
```
Copy the root filesystem to the sdcard, and unmount
```
dd bs=4M if=/dev/mmcblk0p8 of=/media/sdcard/edison-image-edison.ext4
umount /media/sdcard
```
