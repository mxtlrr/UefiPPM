#!/bin/sh

echo Generating FAT image...

dd if=/dev/zero of=uefippm.img bs=512 count=93750
mformat -i uefippm.img ::
mmd -i uefippm.img ::/EFI
mmd -i uefippm.img ::/EFI/BOOT
mcopy -i uefippm.img BOOTX64.efi ::/EFI/BOOT
mcopy -i uefippm.img startup.nsh ::
mcopy -i uefippm.img src/image.ppm ::

echo Done! Time to build the ISO!

mkdir -p iso/
cp uefippm.img iso
xorriso -as mkisofs -R -f -e uefippm.img -no-emul-boot -o uefippm.iso iso || exit

rm -rf iso