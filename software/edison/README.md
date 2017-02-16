Downloading this image requires [git lfs](https://git-lfs.github.com/).

Follow the instructions on that page to install and track the toFlash.tar.gz
file then `git lfs pull` to download.

Then to install:

```
tar -xzf toFlash.tar.gz
cd toFlash
./flashall.sh
```
`flashall.sh` may need to be run as root.

The default username is `user` and the default password is `edison`. Root account access is disabled by default, but `user` has sudo access enabled.

This image has a working sleep mode, and includes serial line ip (slip)
