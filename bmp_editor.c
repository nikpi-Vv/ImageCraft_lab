// bmp_editor.c

#include <stdio.h>
#include "tools.h"

int main(int argn, char *argc[]){

    if (argn < 2){
        printf("Syntax: bmpreader <bmp file>\n");
        return 0;
    }

    const char *image_name = argc[1];

    BMP_Image* bmp_image = parse_bmp_image(image_name);
    
    printf("Width: %u\n", bmp_image->dib_header.width);
    printf("Height: %u\n", bmp_image->dib_header.height);
    printf("Bit depth: %d\n", bmp_image->dib_header.bit_per_pixel);
    printf("Pixel data size: %u bytes\n", bmp_image->dib_header.raw_bitmap_data);

    int data_size = bmp_image->dib_header.height * bmp_image->dib_header.width * bmp_image->dib_header.bit_per_pixel / 8; 
    for (int i = 0; i < data_size; i++){
        if (i == bmp_image->dib_header.width * 3){
            printf("\n");
        }
        printf("%02x ", (unsigned char) bmp_image->data[i]);
    }

    free_bmp_image(bmp_image);
    return 0;
}