#ifndef BMP_TOOLS_H
#define BMP_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char id[2];
    unsigned int size_of_file;
    unsigned int reserved;
    unsigned int pixel_offset;
} BMP_Header;

typedef struct {
    unsigned int size_of_header;
    int width;
    int height;
    unsigned short color_planes;
    unsigned short bit_per_pixel;
    unsigned int compression;
    unsigned int raw_bitmap_data;
    int print_resolution_horizontal;
    int print_resolution_vertical;
    unsigned int color_num;
    unsigned int important_colors;
} DIB_Header;

typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel* pixels;
} ParsedPixelData;

typedef struct BMP_Image {
    BMP_Header bmp_header;
    DIB_Header dib_header;
    unsigned char* data;
    ParsedPixelData pixel_data;
} BMP_Image;

// Парсинг хедеров и данных изображения
BMP_Image* parse_bmp_image(const char* image_name);

// Освобождение памяти данны изображения
void free_bmp_image(BMP_Image* bmp_image);

// Сбор данных пикселей
void parse_pixel_data(BMP_Image* img);

// Обновление данных пикселей
void serialize_pixel_data(BMP_Image* img);

// Сохранение
int save_bmp_image(const char* filename, BMP_Image* img);

#endif