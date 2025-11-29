#ifndef TOOLS_H
#define TOOLS_H

#pragma pack(1) // Выравнивание структур

typedef struct BMP_Header{
    unsigned char id[2];
    unsigned int size_of_file; // 70 байт = 54 байта заголовков + 16 байт данных пикселей
    unsigned char unused[4]; // Зарезервировано для приложений
    unsigned int pixel_offset; // Смещение (в байтах от начала файла), где начинаются данные пикселей. Здесь 54 байта = 14 байт BMP Header + 40 байт DIB Header
} BMP_Header;

typedef struct DIB_Header{
    unsigned int size_of_header; // Размер этого заголовка
    unsigned int width; // Ширина изображения в пикселях
    unsigned int height; // Высота изображения в пикселях
    short color_planes; // Количество цветовых плоскостей
    short bit_per_pixel; // Глубина цвета
    unsigned int compression; // Сжатие, поскольку нет сжатия BI_RGB
    unsigned int raw_bitmap_data; // Размер необработанных данных пикселей, включая выравнивание
    unsigned int print_resolution; // Вертикальное разрешение печати
    unsigned int color_num; // Количество цветов в палитре
    unsigned int inportant_colors; // Количество важных цветов
}DIB_Header;

typedef struct {
    unsigned char r; // Красный
    unsigned char g; // Зелёный
    unsigned char b; // Синий
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel *pixels; // Одномерный массив: pixels[y * width + x]
} ParsedPixelData;

typedef struct BMP_Image{
    BMP_Header bmp_header;
    DIB_Header dib_header;
    char *data;
    ParsedPixelData pixel_data; // Данные о наборе пикселей
}BMP_Image;

#pragma pop

BMP_Image* parse_bmp_image(const char* image_name);
void free_bmp_image(BMP_Image* bmp_image);

#endif