# UefiPPM
A small (>125 LoC!), bare metal, [PPM v6](https://netpbm.sourceforge.net/doc/ppm.html) parser as a UEFI app. It sets up the biggest framebuffer it can, to be able to display
as big of an image as possible. Unfortunately, this does not allow for *all* images
to be displayed.

On QEMU, the app cannot display any images bigger than 2560 x 1600 pixels (4,096,000
pixels).

# Screenshot
<img src="./img/image.png">

# How to use
## Installing POSIX-UEFI
```sh
$ git clone https://gitlab.com/bztsrc/posix-uefi.git
$ cd uefippm/src
$ ln -sf ../../posix-uefi/uefi
```

## Adding your image
Just overwrite `src/image.ppm` with your PPM image.

## Running
```
$ make
...
$ make qemu
```

Or, if you want to boot off of an ISO, you can just run
```
$ make && make iso
```

then flash the ISO to your USB/drive. You can then boot off of it.

# Troubleshooting
The app is meant to tell you what went wrong, for example:
> Sorry, the image is too big for the framebuffer we set up.

or
> This EFI application only runs on PPM v6. Please make sure the image you load uses PPM v6.

This is so you don't have to guess on what's going on.

# TODO
1. Fully parse PPM file (missing maximum channel color value).
2. Optimize width/height detection code.