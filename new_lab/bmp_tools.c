// bmp_tools.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_tools.h"


void read_bmp_row_by_row(FILE* file, BMP_Image* bmp){ // Что делаем? Читаем пиксели и разворачиваем, а надо ли это? да наверное, посмотрим...
    
    int bytes_per_pixel = bmp->dib_header.bit_per_pixel / 8;
    int row_size = bytes_per_pixel * bmp->dib_header.width;

    int row_padding = (4 - (row_size % 4)) % 4; // Количество пикселей для выравнивания

    int row_written = 0; // Количество записанных в файл строк
    unsigned char* row = (unsigned char*)malloc(row_size + row_padding); // Выделение памяти для считывания строк с выравниванием
    unsigned char* p = &bmp->data[(bmp->dib_header.height - 1) * row_size]; // Адрес места в памяти, которое соответствует последней строке, переменная ссылается на первый байт последней строки


    if (fseek(file, bmp->bmp_header.pixel_offset, SEEK_SET) != 0) { // Указатель перемещён на начало чтения файла по смещению на начало пиксельного массива
        printf("Error: failed to seek to pixel data\n");
        free(bmp->data);
        free(bmp);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    while (row_written < bmp->dib_header.height){
        fread(row, row_size + row_padding, 1, file); // Чтение строк из файла
        if (bytes_per_pixel == 3){
            for (int i = 0; i < row_size; i += bytes_per_pixel){ // Читаем по 3 байта из строки и меняем местами
                *p = row[i + 2]; p++; // Движемся по элементам последней строки из переменной data и помещаем туда развернутые тройки, байт, из строки, которая была считана из файла
                *p = row[i + 1]; p++;
                *p = row[i];     p++;
            }
        } else if(bytes_per_pixel == 4){
            for (int i = 0; i < row_size; i += bytes_per_pixel){ // Читаем по 4 байта из строки и меняем местами, по идее хватит и 3, у нас же ргб просто
                *p = row[i + 3]; p++;
                *p = row[i + 2]; p++;
                *p = row[i + 1]; p++;
                *p = row[i];     p++;
            }
        }else{
            printf("Error: don't working with bytes_per_pixel = %d", bytes_per_pixel);
            exit(EXIT_FAILURE);
        }

        row_written++;
        p = p - 2 * row_size;
    }
    
    free(row);
    
}

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

    int data_sixe = bmp->dib_header.width * bmp->dib_header.height * bmp->dib_header.bit_per_pixel / 8;

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

    read_bmp_row_by_row(file, bmp); // little-endian

    #if 0 // big-endian

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
    #endif


    fclose(file);
    return bmp;
}

// Функция освобождения памяти
void free_bmp_image(BMP_Image* bmp_image) {
    if (!bmp_image) return;

    if (bmp_image->data) {
        free(bmp_image->data);
        bmp_image->data = NULL;
    }

    free(bmp_image);
}