#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

BMP_Image* parse_bmp_image(const char* image_name) {
    if (!image_name) {
        printf("Error: image_name is NULL\n");
        return NULL;
    }

    FILE *file = fopen(image_name, "rb");
    if (!file) {
        printf("Error: cannot open file '%s': %s\n", image_name, strerror(errno));
        return NULL;
    }

    // Создаем объект изображения
    BMP_Image *bmp = malloc(sizeof(BMP_Image));
    if (!bmp) {
        printf("Error: out of memory\n");
        fclose(file);
        return NULL;
    }
    memset(bmp, 0, sizeof(BMP_Image));

    // Читаем BMP Header
    if (fread(&bmp->bmp_header, sizeof(BMP_Header), 1, file) != 1) {
        printf("Error: failed to read BMP header\n");
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Проверяем сигнатуру "BM"
    if (bmp->bmp_header.id[0] != 'B' || bmp->bmp_header.id[1] != 'M') {
        printf("Error: not a valid BMP file (missing 'BM' signature)\n");
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Читаем DIB Header
    if (fread(&bmp->dib_header, sizeof(DIB_Header), 1, file) != 1) {
        printf("Error: failed to read DIB header\n");
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Проверяем тип DIB-заголовка (должен быть 40 байт для BITMAPINFOHEADER)
    if (bmp->dib_header.size_of_header != 40) {
        printf("Error: unsupported DIB header size (%u bytes)\n", bmp->dib_header.size_of_header);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Проверяем глубину цвета
    if (bmp->dib_header.bit_per_pixel != 24) {
        printf("Error: only 24-bit BMP supported, got %d bits\n", bmp->dib_header.bit_per_pixel);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Проверяем сжатие
    if (bmp->dib_header.compression != 0) {
        printf("Error: compression not supported (compression type %u)\n", bmp->dib_header.compression);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Выделяем память под данные пикселей
    bmp->data = malloc(bmp->dib_header.raw_bitmap_data);
    if (!bmp->data) {
        printf("Error: out of memory for pixel data\n");
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Переходим к началу данных пикселей
    if (fseek(file, bmp->bmp_header.pixel_offset, SEEK_SET) != 0) {
        printf("Error: failed to seek to pixel data\n");
        free(bmp->data);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Читаем данные пикселей
    if (fread(bmp->data, 1, bmp->dib_header.raw_bitmap_data, file) != bmp->dib_header.raw_bitmap_data) {
        printf("Error: failed to read pixel data\n");
        free(bmp->data);
        free(bmp);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return bmp;
}