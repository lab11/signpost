## Pre-Install

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
*** Ready to receive application ***   <--- It will hang here for ~10s, sadly pushing buttons not yet useful (but not harmful)
Hit any key to stop autoboot:  0  <--- HERE! This timeout is quick! Just mash enter.
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

`flashall.sh` needs to be run as root on Linux and must not be run as root on
mac.

### Installation Issues / FAQ

Unfortunately, this process simply isn't the most robust. Sometimes you just have to try running
things again, sometimes you need to plug/unplug the edison (partiuclarly the flashing interface,
the bottom USB on the Signpost controller board). Sometimes flashall.sh will fail to reboot/catch
the reboot of the Edison - if you're quick, you can just run it again and it'll work.

**Flashing is very slow?** Some USB hubs (even 3.0 hubs) make things slow. We've had a real
mixed bag flashing from inside a virtual machine as well. The best success is to be plugged directly
into USB ports on your machine and flashing from a bare metal OS.

**Well, how long should it take then?** Flashing from bare-metal OS X and Linux machines we find
usually takes around 6 minutes. On the flip side from a Linux VM on a Windows host through a USB
hub took nearly an hour.

**How do I know it's working?** You should leave both terminals open. It starts with flashall
rebooting the edision. Then there are three major flashing steps, but only the last one really
takes a while. Terminal 1 (the serial connection) will be printing `#` characters as it flashes.
It filles somewhere between 2-3 lines on a reasonable width terminal. If `#`'s keep printing, just
keep waiting.

## Post-Install

The default username is `debian` and the default password is `signpost!`. Root
account access is disabled by default, but `debian` has sudo access enabled. SSH
access with password is enabled for the `debian` account.

An `sbuser` account exists with the password `sbuser_default` and is configured with keys from the
[swarmboxadmin](https://www.terraswarm.org/testbeds/wiki/Main/SwarmboxadminGitRepo)
repo in accordance with these
[directions](https://www.icyphy.org/accessors/wiki/Notes/Edison#SetUpSbuser).
SSH access with password is disabled for the `sbuser` account and will only
accept connections from users with the proper SSH key.

### Connect to WiFi:
```
sudo nmcli dev wifi con "ssid" password "ssidpassword"
```

This debian image was built using these [instructions](https://jakehewitt.github.io/custom-edison-image/),
including a patched, working sleep mode, and serial line ip (slip).

This image can be customized further post-install and then [cloned](clone.md) for installation on other Edisons.
