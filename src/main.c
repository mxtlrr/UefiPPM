#include <uefi.h>

#define MAXVAL_ALLOWED 65536

uint64_t fb_addr = 0; uint32_t ppsl = 0;

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
  uintn_t size_mode = sizeof(efi_gop_mode_info_t), currentMode, i;

  status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
  if(!EFI_ERROR(status) && gop) {
    /* we got the interface, get current mode */
    status = gop->QueryMode(gop, gop->Mode ? gop->Mode->Mode : 0, &size_mode, &info);
    if(status == EFI_NOT_STARTED || !gop->Mode) {
      status = gop->SetMode(gop, 0);
    }

    if(EFI_ERROR(status)){
      printf("can't set video mode!\n");
      for(;;); }

    currentMode = gop->Mode->Mode;

    // Get the biggest mode possible (to handle big files)
    int mode = currentMode;
    uint32_t width = 0;
    uint32_t height_gop = 0;
    for(int i = 0; i < gop->Mode->MaxMode; i++){
      status = gop->QueryMode(gop, i, &size_mode, &info);
      if(EFI_ERROR(status)) continue;
      mode = i; // Biggest mode.
      width  = info->HorizontalResolution;
      height_gop = info->VerticalResolution;
    }
    
    // Set mode.
    gop->SetMode(gop, mode);
    
    printf("UefiPPM v1\n");
    printf("Framebuffer address is %d x %d (fmt: WxH)\n\n",
      width, height_gop);

    fb_addr = gop->Mode->FrameBufferBase;
    ppsl = gop->Mode->Information->PixelsPerScanLine;

    // Read PPM file into memory
    FILE* f; unsigned char* buff; long int size = 0;

    if((f = fopen("image.ppm", "r"))){
      fseek(f, 0, SEEK_END);
      size = ftell(f);
      fseek(f, 0, SEEK_SET);

      buff = malloc(size+1); // Allocate the size of our buffer
                            // in terms of memory.
      if(!buff){
        printf("Couldn't allocate memory to read file...\n");
        for(;;);
      }
      fread(buff, size, 1, f);
      fclose(f);
    } else {
      printf("ERR! Cannot load file! Are you sure it's on the FS?\n");
      for(;;);
    }

    // Check if magic bytes are correct
    if(buff[0] != 0x50 || buff[1] != 0x36){
      printf("This EFI application only runs on PPM v6. Please make sure the image you load uses PPM v6.\n");
      for(;;);
    }
    printf("Filesize: %ld bytes.\n", size);


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

    uint64_t widthh  = sum_bytes(bytes, width_digits);
    uint64_t height  = sum_bytes(bytes_height, height_digits);

    printf("File dimensions: %d by %d (px)\n", widthh, height);

    if(widthh > gop->Mode->Information->HorizontalResolution ||
      height  > gop->Mode->Information->VerticalResolution){
      printf("Sorry, the image is too big for the framebuffer we set up.\n");
      printf("Overflow (%d, %d) -> (width, height)\n",
        widthh-(gop->Mode->Information->HorizontalResolution),
        height-(gop->Mode->Information->VerticalResolution));
      for(;;);
    }

    // The maximum color value.
    // Must be less than 65536
    uint8_t bytes_maxval[5]; // Each digit is one byte. 65535 (max number) makes 5 bytes.
    uint32_t counter_maxval = 0;

    // Super cursed but it works!:)
    for(int i = 2+1+width_digits+1+height_digits+1; buff[i] != 0x0a; i++){
      // Note that we do use 0x30 to convert it from ASCII to an actual number.
      bytes_maxval[counter_maxval] = buff[i]-0x30;
      counter_maxval++;
    }

    uint32_t maxval = sum_bytes(bytes_maxval, counter_maxval);
    if(maxval > MAXVAL_ALLOWED){
      printf("PPM spec only allows for Maxval to be %d! You have maxval=%d.\n",
          MAXVAL_ALLOWED, maxval);
      for(;;);
    }
    printf("Maximum color value: %d\n", maxval);

    int x = 0; int y = 0;

    // We start at the first pixel value.
    uint32_t first_pixel_value = (2+1+width_digits+1+height_digits+1+4);
    for(int i = first_pixel_value; i < size; i+=3){
      // buff[i] is the red channel, buff[i+1] is the green channel,
      // buff[i+2] is the blue channel.
      uint32_t color =  (((buff[i] << 8) + buff[i+1]) << 8) + buff[i+2];
      putpixel(x, y, color);

      x++;
      if(x >= widthh){
        x = 0;
        y++;
      }
    }

  } else fprintf(stderr, "unable to get graphics output protocol\n");
  for(;;); // Make sure the user can see what we're drawing, if we return
           // we may boot into some actual operating system.
}
