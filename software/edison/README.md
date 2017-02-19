Downloading this image requires [git lfs](https://git-lfs.github.com/).

Follow the instructions on that page to install and track the toFlash.tar.gz
file then `git lfs pull` to download.

## Installation

You'll need to do thing things in psedo parallel here, so have two terminal windows open.

**Note:** You'll also need to be plugged into both USB ports (one is your serial terminal,
the other is the flash interface).

### Terminal 1 (serial):

You'll need to login to the edison over serial and reboot it. Then your goal it catch it
to enter the bootloader mode. This is a quick timeout, so pay attention as things are
flying by the screen. There's not harm in missing it, just try again. Your session will
look something like this:

```
screen /dev/tty.usbserial-DQ0031Z1 115200
<press enter>
edison login: root
root@edison:~# reboot
         Unmounting /home...
         Unmounting /boot...
[ ... ]
OK   Reached target Shutdown.

******************************
PSH KERNEL VERSION: b0182b2b
		WR: 20104000
******************************
[ ... ]
Hit any key to stop autoboot:  0  <--- HERE! This timeout is quick! Just mash some buttons.
boot >
```
Once we're in the bootloader, we need to set it up to accept a new image:

```
boot > run do_flash
Saving Environment to MMC...
Writing to redundant MMC(0)... done
GADGET DRIVER: usb_dnl_dfu
```

^ There's no time sensitivity here, so this terminal can now just hang out waiting.

> When you're all done (not now), the sequence `Ctrl-a Ctrl-d` will detach your screen session

### Terminal 2 (flashing):

Once your `git lfs pull` completes, you'll have an image and some supporting scripts.
First untar that image, then flash:

```
tar -xzf toFlash.tar.gz
cd toFlash
./flashall.sh
```

`flashall.sh` may need to be run as root.

The default username is `user` and the default password is `edison`. Root account access is disabled by default, but `user` has sudo access enabled.

## Post-Install

### Connect to WiFi:
```
sudo nmcli dev wifi con "ssid" password "ssidpassword"
```

This debian image was built using these [instructions](https://jakehewitt.github.io/custom-edison-image/), including a patched, working sleep mode, and serial line ip (slip).

This image can be customized further post-install and then [cloned](clone.md) for installation on other Edisons.
