// filters.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filters.h"
#include "filter_tools.h"

// Простая реализация: применить фильтры последовательно
void apply_filters(BMP_Image* img, struct FilterNode* filters) {
    if (!img || !filters) return;

    // Сначала парсим пиксели
    parse_pixel_data(img);

    for (struct FilterNode* f = filters; f; f = f->next) {
        if (strcmp(f->name, "crop") == 0) {
            int w = (int)f->params[0];
            int h = (int)f->params[1];
            filter_crop(img, w, h);
        } else if (strcmp(f->name, "gs") == 0) {
            filter_grayscale(img);
        } else if (strcmp(f->name, "neg") == 0) {
            filter_negative(img);
        } else if (strcmp(f->name, "sharp") == 0) {
            filter_sharpen(img);
        } else if (strcmp(f->name, "edge") == 0) {
            filter_edge(img, f->params[0]);
        } else if (strcmp(f->name, "med") == 0) {
            int w = (int)f->params[0];
            if (w % 2 == 0) w++; // делаем нечётным
            filter_median(img, w);
        } else if (strcmp(f->name, "blur") == 0) {
            filter_gaussian_blur(img, f->params[0]);
        } else if (strcmp(f->name, "crystallize") == 0) {
            int bs = (int)(f->params[0] > 0 ? f->params[0] : 16);
            filter_crystallize(img, bs);
        } else if (strcmp(f->name, "glass") == 0) {
            double r = f->params[0] > 0 ? f->params[0] : 5.0;
            filter_glass(img, r);
        } else {
            fprintf(stderr, "Warning: unknown filter '%s' skipped\n", f->name);
        }
    }

    // Обновляем сырые данные
    serialize_pixel_data(img);
}

// ============= Фильтры =============

void filter_crop(BMP_Image* img, int new_w, int new_h) {
    int old_w = img->pixel_data.width;
    int old_h = img->pixel_data.height;
    new_w = (new_w < old_w) ? new_w : old_w;
    new_h = (new_h < old_h) ? new_h : old_h;
    if (new_w <= 0 || new_h <= 0) return;

    Pixel* new_pixels = malloc(new_w * new_h * sizeof(Pixel));
    if (!new_pixels) return;

    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            new_pixels[y * new_w + x] = img->pixel_data.pixels[y * old_w + x];
        }
    }

    free(img->pixel_data.pixels);
    img->pixel_data.pixels = new_pixels;
    img->pixel_data.width = new_w;
    img->pixel_data.height = new_h;

    // Обновляем заголовки
    img->dib_header.width = new_w;
    img->dib_header.height = new_h;
}

void filter_grayscale(BMP_Image* img) {
    int n = img->pixel_data.width * img->pixel_data.height;
    for (int i = 0; i < n; i++) {
        unsigned char gray = (unsigned char)(
            0.299 * img->pixel_data.pixels[i].r +
            0.587 * img->pixel_data.pixels[i].g +
            0.114 * img->pixel_data.pixels[i].b
        );
        img->pixel_data.pixels[i].r = gray;
        img->pixel_data.pixels[i].g = gray;
        img->pixel_data.pixels[i].b = gray;
    }
}

void filter_negative(BMP_Image* img) {
    int n = img->pixel_data.width * img->pixel_data.height;
    for (int i = 0; i < n; i++) {
        img->pixel_data.pixels[i].r = 255 - img->pixel_data.pixels[i].r;
        img->pixel_data.pixels[i].g = 255 - img->pixel_data.pixels[i].g;
        img->pixel_data.pixels[i].b = 255 - img->pixel_data.pixels[i].b;
    }
}

void filter_sharpen(BMP_Image* img) {
    // Ядро: [0 -1 0; -1 5 -1; 0 -1 0]
    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int r = 0, g = 0, b = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    // Клиппинг: ближайший пиксель
                    if (nx < 0) nx = 0;
                    if (ny < 0) ny = 0;
                    if (nx >= w) nx = w - 1;
                    if (ny >= h) ny = h - 1;
                    int idx = ny * w + nx;
                    int weight = 0;
                    if (dx == 0 && dy == 0) weight = 5;
                    else if (abs(dx) + abs(dy) == 1) weight = -1;
                    else weight = 0;
                    r += weight * src[idx].r;
                    g += weight * src[idx].g;
                    b += weight * src[idx].b;
                }
            }
            dst[y * w + x].r = (unsigned char)(r < 0 ? 0 : (r > 255 ? 255 : r));
            dst[y * w + x].g = (unsigned char)(g < 0 ? 0 : (g > 255 ? 255 : g));
            dst[y * w + x].b = (unsigned char)(b < 0 ? 0 : (b > 255 ? 255 : b));
        }
    }
    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
}

// Простейшая реализация edge (через grayscale + порог)
void filter_edge(BMP_Image* img, double threshold) {
    // Сначала в градации серого
    filter_grayscale(img);
    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    // Используем простой оператор: центральный минус соседи
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int sum = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0) nx = 0;
                    if (ny < 0) ny = 0;
                    if (nx >= w) nx = w - 1;
                    if (ny >= h) ny = h - 1;
                    sum += src[ny * w + nx].r;
                }
            }
            int center = src[y * w + x].r;
            int edge = abs(center * 8 - sum); // упрощённый Laplacian
            unsigned char val = (edge / 255.0 > threshold) ? 255 : 0;
            dst[y * w + x].r = dst[y * w + x].g = dst[y * w + x].b = val;
        }
    }
    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
}

void filter_median(BMP_Image* img, int window) {
    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    int half = window / 2;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int* r_vals = malloc(window * window * sizeof(int));
            int* g_vals = malloc(window * window * sizeof(int));
            int* b_vals = malloc(window * window * sizeof(int));
            int idx = 0;
            for (int dy = -half; dy <= half; dy++) {
                for (int dx = -half; dx <= half; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0) nx = 0;
                    if (ny < 0) ny = 0;
                    if (nx >= w) nx = w - 1;
                    if (ny >= h) ny = h - 1;
                    Pixel p = src[ny * w + nx];
                    r_vals[idx] = p.r;
                    g_vals[idx] = p.g;
                    b_vals[idx] = p.b;
                    idx++;
                }
            }
            // Сортируем
            qsort(r_vals, idx, sizeof(int), (int (*)(const void*, const void*)) strcmp);
            qsort(g_vals, idx, sizeof(int), (int (*)(const void*, const void*)) strcmp);
            qsort(b_vals, idx, sizeof(int), (int (*)(const void*, const void*)) strcmp);
            dst[y * w + x].r = (unsigned char)r_vals[idx / 2];
            dst[y * w + x].g = (unsigned char)g_vals[idx / 2];
            dst[y * w + x].b = (unsigned char)b_vals[idx / 2];
            free(r_vals); free(g_vals); free(b_vals);
        }
    }
    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
}

// Простейший Gaussian blur (без оптимизации)
void filter_gaussian_blur(BMP_Image* img, double sigma) {
    if (sigma <= 0) return;
    int kernel_size = (int)(2 * ceil(3 * sigma) + 1);
    if (kernel_size % 2 == 0) kernel_size++;
    int half = kernel_size / 2;

    double* kernel = malloc(kernel_size * kernel_size * sizeof(double));
    double sum = 0.0;
    for (int i = 0; i < kernel_size; i++) {
        for (int j = 0; j < kernel_size; j++) {
            int x = i - half;
            int y = j - half;
            double val = exp(-(x*x + y*y) / (2 * sigma * sigma));
            kernel[i * kernel_size + j] = val;
            sum += val;
        }
    }
    // Нормализация
    for (int i = 0; i < kernel_size * kernel_size; i++) {
        kernel[i] /= sum;
    }

    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) { free(kernel); return; }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            double r = 0, g = 0, b = 0;
            for (int dy = -half; dy <= half; dy++) {
                for (int dx = -half; dx <= half; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0) nx = 0;
                    if (ny < 0) ny = 0;
                    if (nx >= w) nx = w - 1;
                    if (ny >= h) ny = h - 1;
                    double k = kernel[(dy + half) * kernel_size + (dx + half)];
                    Pixel p = src[ny * w + nx];
                    r += k * p.r;
                    g += k * p.g;
                    b += k * p.b;
                }
            }
            dst[y * w + x].r = (unsigned char)(r);
            dst[y * w + x].g = (unsigned char)(g);
            dst[y * w + x].b = (unsigned char)(b);
        }
    }

    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
    free(kernel);
}
#if 0
// ========= Собственные фильтры =========

// Crystallize: усреднение по блокам
void filter_crystallize(BMP_Image* img, int block_size) {
    if (block_size <= 0) block_size = 16;
    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    for (int by = 0; by < h; by += block_size) {
        for (int bx = 0; bx < w; bx += block_size) {
            int count = 0;
            long long sr = 0, sg = 0, sb = 0;
            for (int y = by; y < by + block_size && y < h; y++) {
                for (int x = bx; x < bx + block_size && x < w; x++) {
                    Pixel p = src[y * w + x];
                    sr += p.r;
                    sg += p.g;
                    sb += p.b;
                    count++;
                }
            }
            if (count == 0) continue;
            unsigned char avg_r = (unsigned char)(sr / count);
            unsigned char avg_g = (unsigned char)(sg / count);
            unsigned char avg_b = (unsigned char)(sb / count);
            for (int y = by; y < by + block_size && y < h; y++) {
                for (int x = bx; x < bx + block_size && x < w; x++) {
                    dst[y * w + x].r = avg_r;
                    dst[y * w + x].g = avg_g;
                    dst[y * w + x].b = avg_b;
                }
            }
        }
    }

    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
}

// Glass distortion: случайное смещение в пределах радиуса
void filter_glass(BMP_Image* img, double radius) {
    if (radius <= 0) radius = 5.0;
    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    // Используем простой фиксированный seed для воспроизводимости
    srand(42);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            double angle = ((double)rand() / RAND_MAX) * 2 * M_PI;
            double dist = ((double)rand() / RAND_MAX) * radius;
            int nx = (int)(x + dist * cos(angle));
            int ny = (int)(y + dist * sin(angle));
            // Клиппинг
            if (nx < 0) nx = 0;
            if (ny < 0) ny = 0;
            if (nx >= w) nx = w - 1;
            if (ny >= h) ny = h - 1;
            dst[y * w + x] = src[ny * w + nx];
        }
    }

    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
}
#endif