#include <uefi.h>

efi_physical_address_t fb_addr = 0;
uint32_t ppsl = 0;

void putpixel(int x, int y, uint32_t pixel){
  *((uint32_t*)(fb_addr + 4 * ppsl * y + 4 * x)) = pixel;
}

uint32_t sum_bytes(uint8_t *bytes, size_t n) {
  uint32_t sum = 0;
  for (size_t i = 0; i < n; ++i) sum = sum*10 + bytes[i];
  return sum;
}

int main(int argc, char **argv){
  efi_status_t status;
  efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  efi_gop_t *gop = NULL;
  efi_gop_mode_info_t *info = NULL;
  uintn_t isiz = sizeof(efi_gop_mode_info_t), currentMode, i;

  status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
  if(!EFI_ERROR(status) && gop) {
    /* we got the interface, get current mode */
    status = gop->QueryMode(gop, gop->Mode ? gop->Mode->Mode : 0, &isiz, &info);
    if(status == EFI_NOT_STARTED || !gop->Mode) {
      status = gop->SetMode(gop, 0);
      ST->ConOut->Reset(ST->ConOut, 0);
      ST->StdErr->Reset(ST->StdErr, 0);
    }
    if(EFI_ERROR(status)) {
      printf("can't set video mode\n");
      return 0;
    }
    currentMode = gop->Mode->Mode;

    // Set mode 10, 1280x760
    gop->SetMode(gop, 10);

    fb_addr = gop->Mode->FrameBufferBase;
    ppsl = gop->Mode->Information->PixelsPerScanLine;

    // Read Vro into memory
    // Try and read the kernel
    FILE* f; unsigned char* buff; long int size = 0;

    /// First we need to obviously read the file
    if((f = fopen("image.ppm", "r"))){
      // Get size
      fseek(f, 0, SEEK_END);
      size = ftell(f);
      fseek(f, 0, SEEK_SET);

      // Then read into buff
      buff = malloc(size+1); // Allocate the size of our buffer
                            // in terms of memory.
      if(!buff){
        printf("Couldn't allocate memory to read file...\n");
        return 1;
      }
      fread(buff, size, 1, f);
      fclose(f);
    } else {
      printf("ERR! Cannot load file! Are you sure it's on the FS?\n");
      return 2;
    }

    // Check if magic bytes are correct
    if(buff[0] != 0x50 || buff[1] != 0x36){
      printf("This EFI application only runs on PPM v6. Please make sure the image you load uses PPM v6.\n");
      return 2;
    }
    printf("UefiPPM v1\n\n");
    printf("Filesize: %ld bytes...\n", size);

    // Width
    int width_digits = 0;
    for(int i = 3; buff[i] != 0x20; ++i) width_digits++;
    uint8_t bytes[width_digits];
    for(int i = 3; i != 3+width_digits; i++) bytes[i-3] = buff[i]-0x30;
    
    int height_digits = 0;
    for(int i = 4+width_digits; buff[i] != 0x0a; ++i) height_digits++;
    uint8_t bytes_height[height_digits];
    uint8_t counter = 0;
    for(int i = 3+width_digits+1; i != (3+width_digits+1)+height_digits; i++){
      bytes_height[counter] = buff[i]-0x30;
      counter++;
    }

    uint32_t widthh  = sum_bytes(bytes, width_digits);
    uint32_t height  = sum_bytes(bytes_height, height_digits);

    printf("File dimensions: %d by %d (px)\n", widthh, height);


    int x = 0;
    int y = 0;
    for(int i = 0xf; i < size; i+=3){
      uint8_t r = buff[i];
      uint8_t g = buff[i+1];
      uint8_t b = buff[i+2];

      uint32_t color =  (((r << 8) + g) << 8) + b;
      putpixel(x, y, color);

      x++;
      if(x >= widthh){
        x = 0;
        y++;
      }
    }

  } else fprintf(stderr, "unable to get graphics output protocol\n");
  for(;;);
  return 0;
}
