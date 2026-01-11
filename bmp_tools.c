#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bmp_tools.h"


// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================
// BMP файлы хранят числа в формате little-endian (младший байт первый)
// Ширина (width) - всегда положительная → unsigned int 

// Читает 4 байта из массива data как беззнаковое 32-битное целое число, используется для чтения полей BMP-заголовка

static unsigned int read_uint32(unsigned char* data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    //           0-7 бит       8-15 бит     16-23 бита    24-31 бита
}

// Функция для чтения little-endian чисел (32-битные знаковые)
// Высота (height) - может быть отрицательной (для top-down) → int 

// Читает 4 байта из массива data как знаковое 32-битное целое число, используется для чтения signed-полей в BMP

static int read_int32(unsigned char* data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}


// Функция для чтения little-endian чисел (16-битные беззнаковые)

// Читает 2 байта из массива data как беззнаковое 16-битное целое число, используется для чтения коротких полей BMP
static unsigned short read_uint16(unsigned char* data) {
    return data[0] | (data[1] << 8);
    //           0-7 бит       8-15 бит
}

// Функция для чтения little-endian чисел (16-битные знаковые)
//static short read_int16(unsigned char* data) {
//    return data[0] | (data[1] << 8);
//}

// ==================== ОСНОВНЫЕ ФУНКЦИИ ====================

// Функция: parse_bmp_image
BMP_Image* parse_bmp_image(const char* image_name) {
    if (!image_name) {
        printf("Error: image_name is NULL\n");
        return NULL;
    }

    // Открываем файл в бинарном режиме для чтения
    FILE* file = fopen(image_name, "rb");
    if (!file) {
        printf("Error: cannot open file '%s': %s\n", image_name, strerror(errno));
        return NULL;
    }

    // Читаем первые 54 байта (стандартный заголовок BMP)
    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54) {
        printf("Error: failed to read BMP header\n");
        fclose(file);
        return NULL;
    }

    // Проверяем сигнатуру "BM" (обязательная для BMP файлов)
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Error: not a valid BMP file (missing 'BM' signature)\n");
        fclose(file);
        return NULL;
    }

    // Выделяем память для структуры изображения
    BMP_Image* bmp = malloc(sizeof(BMP_Image));
    if (!bmp) {
        printf("Error: out of memory\n");
        fclose(file);
        return NULL;
    }
    memset(bmp, 0, sizeof(BMP_Image)); // Обнуляем структуру

    // ----- Заполняем BMP Header вручную (little-endian) -----
    // Позиции в заголовке BMP:
    // 0-1: Сигнатура 'BM'
    // 2-5: Размер файла
    // 6-9: Зарезервировано
    // 10-13: Смещение к данным пикселей
    bmp->bmp_header.id[0] = header[0];
    bmp->bmp_header.id[1] = header[1];
    bmp->bmp_header.size_of_file = read_uint32(&header[2]);
    bmp->bmp_header.reserved = read_uint32(&header[6]);
    bmp->bmp_header.pixel_offset = read_uint32(&header[10]);

    // ----- Заполняем DIB Header вручную -----
    // Позиции в DIB заголовке:
    // 14-17: Размер DIB заголовка
    // 18-21: Ширина изображения
    // 22-25: Высота изображения (положительная = bottom-up, отрицательная = top-down)
    // 26-27: Количество цветовых плоскостей (всегда 1)
    // 28-29: Бит на пиксель (24 для RGB)
    // 30-33: Тип сжатия (0 = без сжатия)
    // 34-37: Размер данных пикселей
    // 38-41: Горизонтальное разрешение (пикселей на метр)
    // 42-45: Вертикальное разрешение
    // 46-49: Количество цветов в палитре
    // 50-53: Количество важных цветов
    bmp->dib_header.size_of_header = read_uint32(&header[14]);
    bmp->dib_header.width = read_int32(&header[18]);
    bmp->dib_header.height = read_int32(&header[22]);
    bmp->dib_header.color_planes = read_uint16(&header[26]);
    bmp->dib_header.bit_per_pixel = read_uint16(&header[28]);
    bmp->dib_header.compression = read_uint32(&header[30]);
    bmp->dib_header.raw_bitmap_data = read_uint32(&header[34]);
    bmp->dib_header.print_resolution_horizontal = read_int32(&header[38]);
    bmp->dib_header.print_resolution_vertical = read_int32(&header[42]);
    bmp->dib_header.color_num = read_uint32(&header[46]);
    bmp->dib_header.important_colors = read_uint32(&header[50]);

    // Отладочный вывод информации о загруженном изображении
    printf("Loaded BMP: %dx%d, %d bpp, header size: %u\n", 
           bmp->dib_header.width, bmp->dib_header.height, 
           bmp->dib_header.bit_per_pixel, bmp->dib_header.size_of_header);

    // Проверяем тип DIB-заголовка (BITMAPINFOHEADER = 40 байт)
    if (bmp->dib_header.size_of_header != 40) {
        printf("Warning: DIB header size is %u (expected 40). Trying to continue...\n", 
               bmp->dib_header.size_of_header);
    }

    // Проверяем глубину цвета (поддерживаем только 24-битные BMP)
    if (bmp->dib_header.bit_per_pixel != 24) {
        printf("Error: only 24-bit BMP supported, got %d bits\n", bmp->dib_header.bit_per_pixel);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Проверяем сжатие (поддерживаем только несжатые изображения)
    if (bmp->dib_header.compression != 0) {
        printf("Error: compression not supported (compression type %u)\n", bmp->dib_header.compression);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Вычисляем размер данных если не указан в заголовке
    if (bmp->dib_header.raw_bitmap_data == 0) {
        int bytes_per_pixel = bmp->dib_header.bit_per_pixel / 8; // 24 бита = 3 байта
        int row_size = bytes_per_pixel * bmp->dib_header.width;
        // Выравнивание строк до кратного 4 байтам (требование BMP формата)
        int row_padding = (4 - (row_size % 4)) % 4;
        bmp->dib_header.raw_bitmap_data = (row_size + row_padding) * abs(bmp->dib_header.height);
        printf("Calculated data size: %u bytes\n", bmp->dib_header.raw_bitmap_data);
    }

    // Выделяем память под данные пикселей
    bmp->data = malloc(bmp->dib_header.raw_bitmap_data);
    if (!bmp->data) {
        printf("Error: out of memory for pixel data (%u bytes)\n", bmp->dib_header.raw_bitmap_data);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Переходим к началу данных пикселей в файле
    if (fseek(file, bmp->bmp_header.pixel_offset, SEEK_SET) != 0) {
        printf("Error: failed to seek to pixel data (offset: %u)\n", bmp->bmp_header.pixel_offset);
        free(bmp->data);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Читаем данные пикселей из файла
    size_t bytes_read = fread(bmp->data, 1, bmp->dib_header.raw_bitmap_data, file);
    if (bytes_read != bmp->dib_header.raw_bitmap_data) {
        printf("Error: failed to read pixel data (read %zu of %u bytes)\n", 
               bytes_read, bmp->dib_header.raw_bitmap_data);
        free(bmp->data);
        free(bmp);
        fclose(file);
        return NULL;
    }

    // Закрываем файл и возвращаем успешно загруженное изображение
    fclose(file);
    printf("Successfully loaded BMP: %dx%d, %u bytes of pixel data\n",
           bmp->dib_header.width, bmp->dib_header.height, bmp->dib_header.raw_bitmap_data);
    return bmp;
}
// Освобождение памяти, занятой изображением
void free_bmp_image(BMP_Image* bmp_image) {
    if (!bmp_image) return;

    // Освобождаем данные пикселей в сыром формате (BGR)
    if (bmp_image->data) {
        free(bmp_image->data);
    }

    // Освобождаем распарсенные данные пикселей (RGB)
    if (bmp_image->pixel_data.pixels) {
        free(bmp_image->pixel_data.pixels);
    }

    // Освобождаем саму структуру
    free(bmp_image);
}


// Конвертация сырых данных (BGR, bottom-up) в структурированные (RGB, top-down)
void parse_pixel_data(BMP_Image* img) {
    if (!img || !img->data) return;
    
    int width = img->dib_header.width;
    int height = abs(img->dib_header.height); // Берем абсолютное значение высоты
    int bytes_per_pixel = img->dib_header.bit_per_pixel / 8; // 3 для 24-битных BMP
    int row_size = bytes_per_pixel * width;
    // Вычисляем padding для выравнивания строк
    int row_padding = (4 - (row_size % 4)) % 4;
    
    // Если уже есть распарсенные данные - освобождаем их
    if (img->pixel_data.pixels) {
        free(img->pixel_data.pixels);
    }
    
    // Выделяем память под структурированные пиксели
    img->pixel_data.width = width;
    img->pixel_data.height = height;
    img->pixel_data.pixels = malloc(width * height * sizeof(Pixel));
    
    if (!img->pixel_data.pixels) {
        printf("Error: out of memory in parse_pixel_data\n");
        return;
    }
    
    // Определяем направление строк в исходных данных
    // Если height положительное - bottom-up (строки снизу вверх)
    // Если height отрицательное - top-down (строки сверху вниз)
    int is_top_down = (img->dib_header.height < 0);
    
    // Конвертируем данные: из формата BGR в RGB, с учетом направления строк
    for (int y = 0; y < height; y++) {
        // Вычисляем индекс строки в исходных данных
        int src_row;
        if (is_top_down) {
            src_row = y; // top-down - строки идут по порядку
        } else {
            src_row = height - 1 - y; // bottom-up - строки в обратном порядке
        }
        
        // Смещение начала строки в сырых данных
        int src_row_offset = src_row * (row_size + row_padding);
        
        // Обрабатываем каждый пиксель в строке
        for (int x = 0; x < width; x++) {
            int src_idx = src_row_offset + x * bytes_per_pixel; // Индекс в сырых данных
            int dst_idx = y * width + x; // Индекс в структурированных данных
            
            // BGR -> RGB (BMP хранит цвета в порядке B, G, R)
            img->pixel_data.pixels[dst_idx].b = img->data[src_idx];
            img->pixel_data.pixels[dst_idx].g = img->data[src_idx + 1];
            img->pixel_data.pixels[dst_idx].r = img->data[src_idx + 2];
        }
    }
}

// Обратная конвертация структурированных данных (RGB, top-down) в сырые (BGR, bottom-up)
void serialize_pixel_data(BMP_Image* img) {
    if (!img || !img->pixel_data.pixels) return;
    
    int width = img->pixel_data.width;
    int height = img->pixel_data.height;
    int bytes_per_pixel = 3; // 24 бита = 3 байта (B, G, R)
    int row_size = bytes_per_pixel * width;
    int row_padding = (4 - (row_size % 4)) % 4;
    int raw_bitmap_data = (row_size + row_padding) * height;
    
    // Обновляем заголовки в соответствии с новыми размерами
    img->dib_header.width = width;
    img->dib_header.height = height; // положительное = bottom-up формат
    img->dib_header.bit_per_pixel = 24;
    img->dib_header.raw_bitmap_data = raw_bitmap_data;
    img->bmp_header.size_of_file = 54 + raw_bitmap_data; // 14 (BMP header) + 40 (DIB header) + данные
    
    // Перевыделяем память под сырые данные если нужно
    if (img->data) {
        free(img->data);
    }
    img->data = malloc(raw_bitmap_data);
    
    if (!img->data) {
        printf("Error: out of memory in serialize_pixel_data\n");
        return;
    }
    
    // Конвертируем данные: из формата RGB в BGR, с конвертацией в bottom-up
    for (int y = 0; y < height; y++) {
        int dst_row = height - 1 - y; // bottom-up - последняя строка в файле = первая в данных
        int dst_row_offset = dst_row * (row_size + row_padding);
        
        for (int x = 0; x < width; x++) {
            int src_idx = y * width + x; // Индекс в структурированных данных
            int dst_idx = dst_row_offset + x * bytes_per_pixel; // Индекс в сырых данных
            
            // RGB -> BGR и сохранение
            img->data[dst_idx] = img->pixel_data.pixels[src_idx].b;     // Синий канал
            img->data[dst_idx + 1] = img->pixel_data.pixels[src_idx].g; // Зеленый канал
            img->data[dst_idx + 2] = img->pixel_data.pixels[src_idx].r; // Красный канал
        }
        
        // Добавляем padding (выравнивание до 4 байт) нулями
        for (int p = 0; p < row_padding; p++) {
            img->data[dst_row_offset + row_size + p] = 0;
        }
    }
}


// Сохранение BMP изображения в файл
int save_bmp_image(const char* filename, BMP_Image* img) {
    if (!filename || !img) {
        printf("Error: invalid arguments to save_bmp_image\n");
        return 0;
    }
    
    // Открываем файл для записи в бинарном режиме
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: cannot open file '%s' for writing\n", filename);
        return 0;
    }
    
    // Обновляем сырые данные перед сохранением
    serialize_pixel_data(img);
    
    // Подготавливаем заголовок в байтовом массиве
    unsigned char header[54] = {0};
    
    // ----- Заполняем BMP Header -----
    header[0] = 'B';
    header[1] = 'M';
    *(unsigned int*)&header[2] = img->bmp_header.size_of_file;  // Размер файла
    *(unsigned int*)&header[6] = 0;  // Зарезервировано
    *(unsigned int*)&header[10] = 54; // Смещение к данным (14 + 40)
    
    // ----- Заполняем DIB Header -----
    *(unsigned int*)&header[14] = 40; // Размер DIB заголовка (BITMAPINFOHEADER)
    *(int*)&header[18] = img->dib_header.width;  // Ширина
    *(int*)&header[22] = img->dib_header.height; // Высота
    *(unsigned short*)&header[26] = 1;  // Цветовые плоскости (всегда 1)
    *(unsigned short*)&header[28] = 24; // Бит на пиксель (24 = RGB)
    *(unsigned int*)&header[30] = 0;  // Сжатие (0 = без сжатия)
    *(unsigned int*)&header[34] = img->dib_header.raw_bitmap_data; // Размер данных
    *(int*)&header[38] = 2835; // Горизонтальное разрешение (72 DPI = 2835 пикселей/метр)
    *(int*)&header[42] = 2835; // Вертикальное разрешение (72 DPI)
    *(unsigned int*)&header[46] = 0; // Цвета в палитре (0 = все)
    *(unsigned int*)&header[50] = 0; // Важные цвета (0 = все)
    
    // Записываем заголовок в файл
    if (fwrite(header, 1, 54, file) != 54) {
        printf("Error: failed to write header\n");
        fclose(file);
        return 0;
    }
    
    // Записываем данные пикселей в файл
    if (fwrite(img->data, 1, img->dib_header.raw_bitmap_data, file) != img->dib_header.raw_bitmap_data) {
        printf("Error: failed to write pixel data\n");
        fclose(file);
        return 0;
    }
    
    // Закрываем файл
    fclose(file);
    printf("Successfully saved BMP: %dx%d, %u bytes\n",
           img->dib_header.width, img->dib_header.height, img->dib_header.raw_bitmap_data);
    return 1;
}
