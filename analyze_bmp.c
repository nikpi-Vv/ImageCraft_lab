#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE* f = fopen("lenna.bmp", "rb");
    if (!f) {
        printf("Cannot open lenna.bmp\n");
        return 1;
    }
    
    // ?????? ?????? 100 ????
    unsigned char buffer[100];
    fread(buffer, 1, 100, f);
    fclose(f);
    
    printf("First 100 bytes of lenna.bmp (hex):\n");
    for (int i = 0; i < 100; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n\n");
    
    // ??????????? ?????????
    printf("BMP Signature: %c%c\n", buffer[0], buffer[1]);
    
    // ?????? ????? (little-endian)
    unsigned int file_size = buffer[2] | (buffer[3] << 8) | (buffer[4] << 16) | (buffer[5] << 24);
    printf("File size: %u bytes (%.2f MB)\n", file_size, file_size / (1024.0 * 1024.0));
    
    // ???????? ? ??????
    unsigned int data_offset = buffer[10] | (buffer[11] << 8) | (buffer[12] << 16) | (buffer[13] << 24);
    printf("Data offset: %u\n", data_offset);
    
    // ?????? DIB ?????????
    unsigned int dib_size = buffer[14] | (buffer[15] << 8) | (buffer[16] << 16) | (buffer[17] << 24);
    printf("DIB header size: %u\n", dib_size);
    
    // ??????
    int width = buffer[18] | (buffer[19] << 8) | (buffer[20] << 16) | (buffer[21] << 24);
    printf("Width: %d\n", width);
    
    // ??????
    int height = buffer[22] | (buffer[23] << 8) | (buffer[24] << 16) | (buffer[25] << 24);
    printf("Height: %d\n", height);
    
    // ??? ?? ???????
    short bpp = buffer[28] | (buffer[29] << 8);
    printf("Bits per pixel: %d\n", bpp);
    
    // ??????
    unsigned int compression = buffer[30] | (buffer[31] << 8) | (buffer[32] << 16) | (buffer[33] << 24);
    printf("Compression: %u\n", compression);
    
    return 0;
}
