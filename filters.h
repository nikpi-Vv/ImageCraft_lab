#ifndef FILTERS_H
#define FILTERS_H

#include "bmp_tools.h"
#include "filter_tools.h" 

// Последовательное применяет фильтры
void apply_filters(BMP_Image* img, FilterNode* filters);

// Обрезает изображение до указанного размера (верхний левый угол).
void filter_crop(BMP_Image* img, int w, int h);

// Преобразует изображение в оттенки серого по формуле: 0.299·R + 0.587·G + 0.114·B
void filter_grayscale(BMP_Image* img);

// Инвертирует цвета: каждый канал заменяется на 255 − значение.
void filter_negative(BMP_Image* img);

// Повышает резкость методом Unsharp Mask: усиливает края за счёт вычитания размытой копии.
void filter_sharpen(BMP_Image* img);

// Выделяет границы: преобразует в серый, применяет лапласиан и бинаризует по порогу (0.0–1.0).
void filter_edge(BMP_Image* img, double threshold);

// Медианный фильтр: удаляет импульсный шум ("соль и перец"), заменяя пиксель на медиану окрестности.
void filter_median(BMP_Image* img, int window);

// Размывает изображение с помощью разделимого box-фильтра. Чем больше сигма — тем сильнее размытие.
void filter_gaussian_blur(BMP_Image* img, double sigma);

// Искажает изображение, как через стекло: каждый пиксель заменяется на случайный из окружности заданного радиуса.
void filter_glass(BMP_Image* img, double radius);

// Создаёт "кристаллы": разбивает изображение на блоки и заполняет каждый усреднённым цветом.
void filter_crystallize(BMP_Image* img, int cell_size);

// Собирает исходное изображение из маленьких картинок (CIFAR-100), подбирая их по среднему цвету блока.
void filter_mosaic_cifar(BMP_Image* img);

#endif