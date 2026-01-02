// filters.h

#ifndef FILTERS_H
#define FILTERS_H

#include "bmp_tools.h"


void apply_filters(BMP_Image* img, struct FilterNode* filters);

// Вспомогательные функции фильтров (можно сделать static в .c)
void filter_crop(BMP_Image* img, int w, int h);
void filter_grayscale(BMP_Image* img);
void filter_negative(BMP_Image* img);
void filter_sharpen(BMP_Image* img);
void filter_edge(BMP_Image* img, double threshold);
void filter_median(BMP_Image* img, int window);
void filter_gaussian_blur(BMP_Image* img, double sigma);

// Собственные фильтры
void filter_crystallize(BMP_Image* img, int block_size);
void filter_glass(BMP_Image* img, double radius);

#endif