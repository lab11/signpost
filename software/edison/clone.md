### Clone a live Edision Image

Become root user:
```
sudo su
```

Create a mounting point for the sdcard:
```
mkdir /media/sdcard
```

Perform an emergency remount of the root filesystem
```
echo u > /proc/sysrq-trigger
```

Mount the sdcard
```
mount /dev/mmcblk1p1 /media/sdcard
```

Copy the root filesystem to the sdcard, and unmount
```
dd bs=4M if=/dev/mmcblk0p8 | pv | of=/media/sdcard/signpost-X.X.X.edison.root
umount /media/sdcard
```
Where `X.X.X` is the version you've deemed this image.

Replace the edison image in `toFlash/` with your newly generated image and be
sure to update `toFlash/flashall.sh` with your image name:

```
echo "Flashing rootfs, (it can take up to 5 minutes... Please be patient)"
flash-command --alt rootfs -D "${ESC_BASE_DIR}/signpost-X.X.X.edison.root" -R
```
