@echo -off
cls
if exist .\efi\boot\BOOTX64.efi then
 .\efi\boot\BOOTX64.efi
 goto END
endif

if exist fs0:\efi\boot\BOOTX64.efi then
 fs0:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs1:\efi\boot\BOOTX64.efi then
 fs1:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs2:\efi\boot\BOOTX64.efi then
 fs2:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs3:\efi\boot\BOOTX64.efi then
 fs3:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs4:\efi\boot\BOOTX64.efi then
 fs4:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs5:\efi\boot\BOOTX64.efi then
 fs5:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs6:\efi\boot\BOOTX64.efi then
 fs6:
 efi\boot\BOOTX64.efi
 goto END
endif

if exist fs7:\efi\boot\BOOTX64.efi then
 fs7:
 efi\boot\BOOTX64.efi
 goto END
endif

 
:END