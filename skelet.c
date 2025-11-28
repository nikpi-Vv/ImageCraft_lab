/*image_craft/
  include/
    color.h
    image.h
    bmp.h
    filters.h
    pipeline.h
  src/
    main.c
    color.c
    image.c
    bmp.c
    filters_basic.c
    filters_extra.c
    pipeline.c
  Makefile
  README.md
*/

//!!! здесь нет ни строчки моего текста, АТЕНШИОН, 
//сделано с помощью ИИ, он может ошибаться и тд

/*// include/color.h
#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} Rgb;

#endif // COLOR_H
*/

/*
// include/image.h
#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include "color.h"

typedef struct {
    uint32_t width;
    uint32_t height;
    Rgb *pixels; // width * height последовательно, row-major
} Image;

int image_alloc(Image *img, uint32_t width, uint32_t height);
void image_free(Image *img);

#endif // IMAGE_H
*/

/*
// include/bmp.h
#ifndef BMP_H
#define BMP_H

#include "image.h"

int bmp_read(const char *filename, Image *out);
int bmp_write(const char *filename, const Image *img);

#endif // BMP_H

*/

/*// include/filters.h
#ifndef FILTERS_H
#define FILTERS_H

#include "image.h"

// базовые фильтры
int filter_crop(Image *img, uint32_t x, uint32_t y,
                uint32_t w, uint32_t h);
int filter_grayscale(Image *img);
int filter_negative(Image *img);
int filter_sharpen(Image *img);
int filter_edge(Image *img, uint8_t threshold);
int filter_median(Image *img, uint32_t window);
int filter_gaussian_blur(Image *img, double sigma);

// сюда же потом добавите свои фильтры
// int filter_custom_1(Image *img, ...);
// int filter_custom_2(Image *img, ...);

#endif // FILTERS_H
*/

/*
// include/pipeline.h
#ifndef PIPELINE_H
#define PIPELINE_H

#include "image.h"

// Разбирает argv, применяет все фильтры по порядку к img.
int run_pipeline(Image *img, int argc, char **argv, int arg_start);

#endif // PIPELINE_H
*/

/*// src/main.c
#include <stdio.h>
#include "bmp.h"
#include "pipeline.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.bmp output.bmp [filters...]\n", argv[0]);
        return 1;
    }

    const char *input  = argv[1];
    const char *output = argv[2];

    Image img = {0};
    if (bmp_read(input, &img) != 0) {
        fprintf(stderr, "Failed to read input file\n");
        return 1;
    }

    if (run_pipeline(&img, argc, argv, 3) != 0) {
        fprintf(stderr, "Failed to apply filters\n");
        image_free(&img);
        return 1;
    }

    if (bmp_write(output, &img) != 0) {
        fprintf(stderr, "Failed to write output file\n");
        image_free(&img);
        return 1;
    }

    image_free(&img);
    return 0;
}
*/