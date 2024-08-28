#!/bin/bash
bash makeiso.sh

qemu-system-x86_64 -cpu qemu64 \
  -drive if=pflash,format=raw,unit=0,file=ovmf/OVMF_CODE.fd,readonly=on \
  -drive if=pflash,format=raw,unit=1,file=ovmf/OVMF_VARS.fd -drive file=uefippm.img,if=ide \
  -net none
