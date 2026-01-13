#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filters.h"

void apply_filters(BMP_Image* img, FilterNode* filters) {
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
            if (w % 2 == 0) w++;
            filter_median(img, w);
        } else if (strcmp(f->name, "blur") == 0) {
            filter_gaussian_blur(img, f->params[0]);
        } else if (strcmp(f->name, "crystallize") == 0) {
            int bs = (int)f->params[0];
            filter_crystallize(img, bs);
        } else if (strcmp(f->name, "glass") == 0) {
            double r = f->params[0];
            filter_glass(img, r);
        } else if (strcmp(f->name, "mosaic") == 0) {
            filter_mosaic(img);
        } else {
            printf("Warning: unknown filter '%s' skipped\n", f->name);
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
    if (!img || !img->pixel_data.pixels || window < 1) return;
    if (window % 2 == 0) window++;

    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    int half = window / 2;
    int total = window * window;

    // Гистограммы для каждого канала (256 ячеек)
    int r_hist[256] = {0};
    int g_hist[256] = {0};
    int b_hist[256] = {0};

    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    // Обработка каждой строки
    for (int y = 0; y < h; y++) {
        // Сброс гистограмм перед началом строки
        memset(r_hist, 0, sizeof(r_hist));
        memset(g_hist, 0, sizeof(g_hist));
        memset(b_hist, 0, sizeof(b_hist));

        // Инициализация первого окна в строке (x = 0)
        for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
                int nx = dx;
                int ny = y + dy;
                if (nx < 0) nx = 0;
                if (ny < 0) ny = 0;
                if (nx >= w) nx = w - 1;
                if (ny >= h) ny = h - 1;
                Pixel p = src[ny * w + nx];
                r_hist[p.r]++;
                g_hist[p.g]++;
                b_hist[p.b]++;
            }
        }

        // Обработка всех x в строке
        for (int x = 0; x < w; x++) {
            // Найти медиану по гистограмме
            int count = 0;
            unsigned char median_r = 0, median_g = 0, median_b = 0;
            int target = (total + 1) / 2; // медиана = элемент №target

            for (int i = 0; i < 256; i++) {
                count += r_hist[i];
                if (count >= target) { median_r = i; break; }
            }
            count = 0;
            for (int i = 0; i < 256; i++) {
                count += g_hist[i];
                if (count >= target) { median_g = i; break; }
            }
            count = 0;
            for (int i = 0; i < 256; i++) {
                count += b_hist[i];
                if (count >= target) { median_b = i; break; }
            }

            dst[y * w + x].r = median_r;
            dst[y * w + x].g = median_g;
            dst[y * w + x].b = median_b;

            // Обновление гистограммы: убрать левый столбец, добавить правый
            if (x + half + 1 < w) { // если есть следующий столбец
                // Удалить столбец (x - half)
                int col_out = x - half;
                if (col_out < 0) col_out = 0;
                for (int dy = -half; dy <= half; dy++) {
                    int ny = y + dy;
                    if (ny < 0) ny = 0;
                    if (ny >= h) ny = h - 1;
                    Pixel p = src[ny * w + col_out];
                    r_hist[p.r]--;
                    g_hist[p.g]--;
                    b_hist[p.b]--;
                }

                // Добавить столбец (x + half + 1)
                int col_in = x + half + 1;
                for (int dy = -half; dy <= half; dy++) {
                    int ny = y + dy;
                    if (ny < 0) ny = 0;
                    if (ny >= h) ny = h - 1;
                    Pixel p = src[ny * w + col_in];
                    r_hist[p.r]++;
                    g_hist[p.g]++;
                    b_hist[p.b]++;
                }
            }
        }
    }

    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;
}

void filter_gaussian_blur(BMP_Image* img, double sigma) {
    if (!img || !img->pixel_data.pixels || sigma <= 0.0) return;

    // Преобразуем sigma в радиус окна (эмпирически)
    int radius = (int)(sigma * 2.0); // например: sigma=1.0 → radius=2
    if (radius < 1) radius = 1;

    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;

    // Временные буферы
    Pixel* temp = malloc(w * h * sizeof(Pixel));
    Pixel* dst  = malloc(w * h * sizeof(Pixel));
    if (!temp || !dst) {
        free(temp); free(dst);
        return;
    }

    // === Проход 1: горизонтальное усреднение ===
    for (int y = 0; y < h; y++) {
        int window_size = 2 * radius + 1;
        long sum_r = 0, sum_g = 0, sum_b = 0;

        // Инициализация первого окна в строке
        for (int x = -radius; x <= radius; x++) {
            int nx = (x < 0) ? 0 : (x >= w ? w - 1 : x);
            Pixel p = src[y * w + nx];
            sum_r += p.r;
            sum_g += p.g;
            sum_b += p.b;
        }
        temp[y * w + 0].r = (unsigned char)(sum_r / window_size);
        temp[y * w + 0].g = (unsigned char)(sum_g / window_size);
        temp[y * w + 0].b = (unsigned char)(sum_b / window_size);

        // Скользящее окно по строке
        for (int x = 1; x < w; x++) {
            // Убрать левый пиксель
            int out_x = x - radius - 1;
            if (out_x >= 0) {
                Pixel p_out = src[y * w + out_x];
                sum_r -= p_out.r;
                sum_g -= p_out.g;
                sum_b -= p_out.b;
            } else {
                // Клиппинг: левый край — используем пиксель 0
                Pixel p_out = src[y * w + 0];
                sum_r -= p_out.r;
                sum_g -= p_out.g;
                sum_b -= p_out.b;
            }

            // Добавить правый пиксель
            int in_x = x + radius;
            if (in_x < w) {
                Pixel p_in = src[y * w + in_x];
                sum_r += p_in.r;
                sum_g += p_in.g;
                sum_b += p_in.b;
            } else {
                // Клиппинг: правый край — используем последний пиксель
                Pixel p_in = src[y * w + (w - 1)];
                sum_r += p_in.r;
                sum_g += p_in.g;
                sum_b += p_in.b;
            }

            temp[y * w + x].r = (unsigned char)(sum_r / window_size);
            temp[y * w + x].g = (unsigned char)(sum_g / window_size);
            temp[y * w + x].b = (unsigned char)(sum_b / window_size);
        }
    }

    // === Проход 2: вертикальное усреднение ===
    for (int x = 0; x < w; x++) {
        int window_size = 2 * radius + 1;
        long sum_r = 0, sum_g = 0, sum_b = 0;

        // Инициализация первого окна в столбце
        for (int y = -radius; y <= radius; y++) {
            int ny = (y < 0) ? 0 : (y >= h ? h - 1 : y);
            Pixel p = temp[ny * w + x];
            sum_r += p.r;
            sum_g += p.g;
            sum_b += p.b;
        }
        dst[0 * w + x].r = (unsigned char)(sum_r / window_size);
        dst[0 * w + x].g = (unsigned char)(sum_g / window_size);
        dst[0 * w + x].b = (unsigned char)(sum_b / window_size);

        // Скользящее окно по столбцу
        for (int y = 1; y < h; y++) {
            // Убрать верхний пиксель
            int out_y = y - radius - 1;
            if (out_y >= 0) {
                Pixel p_out = temp[out_y * w + x];
                sum_r -= p_out.r;
                sum_g -= p_out.g;
                sum_b -= p_out.b;
            } else {
                Pixel p_out = temp[0 * w + x];
                sum_r -= p_out.r;
                sum_g -= p_out.g;
                sum_b -= p_out.b;
            }

            // Добавить нижний пиксель
            int in_y = y + radius;
            if (in_y < h) {
                Pixel p_in = temp[in_y * w + x];
                sum_r += p_in.r;
                sum_g += p_in.g;
                sum_b += p_in.b;
            } else {
                Pixel p_in = temp[(h - 1) * w + x];
                sum_r += p_in.r;
                sum_g += p_in.g;
                sum_b += p_in.b;
            }

            dst[y * w + x].r = (unsigned char)(sum_r / window_size);
            dst[y * w + x].g = (unsigned char)(sum_g / window_size);
            dst[y * w + x].b = (unsigned char)(sum_b / window_size);
        }
    }

    // Замена данных
    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;

    free(temp);
}

void filter_glass(BMP_Image* img, double radius) {
    if (!img || !img->pixel_data.pixels || radius <= 0.0) return;

    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    // Фиксированное зерно для воспроизводимости
    unsigned int seed = 12345;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            // Генератор псевдослучайных чисел (LCG)
            seed = seed * 1103515245 + 12345;
            double rand_x = (double)(seed % 65536) / 65536.0; // [0,1)
            seed = seed * 1103515245 + 12345;
            double rand_y = (double)(seed % 65536) / 65536.0;

            // Смещение в круге радиуса `radius`
            double angle = rand_x * 2.0 * 3.1415;
            double dist = radius * sqrt(rand_y); // равномерно в круге
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

void filter_crystallize(BMP_Image* img, int cell_size) {
    if (!img || !img->pixel_data.pixels || cell_size < 1) return;

    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) return;

    // Проходим по ячейкам
    for (int cy = 0; cy < h; cy += cell_size) {
        for (int cx = 0; cx < w; cx += cell_size) {
            // Границы ячейки
            int x_end = (cx + cell_size < w) ? cx + cell_size : w;
            int y_end = (cy + cell_size < h) ? cy + cell_size : h;

            // Считаем сумму
            long sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;

            for (int y = cy; y < y_end; y++) {
                for (int x = cx; x < x_end; x++) {
                    Pixel p = src[y * w + x];
                    sum_r += p.r;
                    sum_g += p.g;
                    sum_b += p.b;
                    count++;
                }
            }

            // Усреднённый цвет
            unsigned char avg_r = (unsigned char)(sum_r / count);
            unsigned char avg_g = (unsigned char)(sum_g / count);
            unsigned char avg_b = (unsigned char)(sum_b / count);

            // Заполняем ячейку
            for (int y = cy; y < y_end; y++) {
                for (int x = cx; x < x_end; x++) {
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


// ============= Мозайка =============

typedef struct {
    unsigned char* data;      // все данные из bin-файла
    int count;                // количество изображений
    int width;                // 32
    int height;               // 32
} CIFAR_Dataset;

typedef struct {
    unsigned char r, g, b;  // средний цвет
    int index;              // индекс в датасете
} TileInfo;

CIFAR_Dataset* load_dataset(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    // Определим размер файла
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    // Каждое изображение: 3073 байта (1 метка + 3072 пикселей)
    int image_size = 3074;
    int count = size / image_size;
    if (count * image_size != size) {
        fclose(f);
        return NULL;
    }

    CIFAR_Dataset* ds = malloc(sizeof(CIFAR_Dataset));
    if (!ds) { fclose(f); return NULL; }

    ds->data = malloc(size);
    if (!ds->data) { free(ds); fclose(f); return NULL; }

    fread(ds->data, 1, size, f);
    fclose(f);

    ds->count = count;
    ds->width = 32;
    ds->height = 32;

    return ds;
}

void free_dataset(CIFAR_Dataset* ds) {
    if (ds) {
        free(ds->data);
        free(ds);
    }
}

static double color_distance(unsigned char r1, unsigned char g1, unsigned char b1,
                             unsigned char r2, unsigned char g2, unsigned char b2) {
    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;
    return dr*dr + dg*dg + db*db; // квадрат расстояния — быстрее sqrt
}

void filter_mosaic(BMP_Image* img) {

    // Загружаем датасет
    CIFAR_Dataset* ds = load_dataset("cifar-100-binary/train.bin");
    if (!ds) {
        printf("Error: failed to load cifar-100-binary/train.bin\n");
        return;
    }

    int w = img->pixel_data.width;
    int h = img->pixel_data.height;
    Pixel* src = img->pixel_data.pixels;
    Pixel* dst = malloc(w * h * sizeof(Pixel));
    if (!dst) {
        free_dataset(ds);
        return;
    }

    // Предвычисляем средние цвета всех тайлов
    TileInfo* tiles = malloc(ds->count * sizeof(TileInfo));
    if (!tiles) {
        free_dataset(ds);
        free(dst);
        return;
    }

    for (int i = 0; i < ds->count; i++) {
        unsigned char* data = ds->data + i * 3074 + 2; // пропускаем 2 метки
        long sum_r = 0, sum_g = 0, sum_b = 0;
        for (int j = 0; j < 1024; j++) { // R канал
            sum_r += data[j];
        }
        for (int j = 0; j < 1024; j++) { // G канал
            sum_g += data[1024 + j];
        }
        for (int j = 0; j < 1024; j++) { // B канал
            sum_b += data[2048 + j];
        }
        tiles[i].r = (unsigned char)(sum_r / 1024);
        tiles[i].g = (unsigned char)(sum_g / 1024);
        tiles[i].b = (unsigned char)(sum_b / 1024);
        tiles[i].index = i;
    }

    // Обработка каждого блока
    for (int y = 0; y < h; y += 32) {
        for (int x = 0; x < w; x += 32) {
            // Вычисляем средний цвет блока
            long sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;
            for (int dy = 0; dy < 32 && (y + dy) < h; dy++) {
                for (int dx = 0; dx < 32 && (x + dx) < w; dx++) {
                    Pixel p = src[(y + dy) * w + (x + dx)];
                    sum_r += p.r;
                    sum_g += p.g;
                    sum_b += p.b;
                    count++;
                }
            }
            unsigned char avg_r = (unsigned char)(sum_r / count);
            unsigned char avg_g = (unsigned char)(sum_g / count);
            unsigned char avg_b = (unsigned char)(sum_b / count);

            // Находим ближайший тайл
            int best_idx = 0;
            double min_dist = color_distance(avg_r, avg_g, avg_b,
                                             tiles[0].r, tiles[0].g, tiles[0].b);

            for (int i = 1; i < ds->count; i++) {
                double dist = color_distance(avg_r, avg_g, avg_b,
                                            tiles[i].r, tiles[i].g, tiles[i].b);
                if (dist < min_dist) {
                    min_dist = dist;
                    best_idx = i;
                }
            }

            // Вставляем выбранный тайл
            unsigned char* tile_data = ds->data + tiles[best_idx].index * 3074 + 2;
            for (int dy = 0; dy < 32 && (y + dy) < h; dy++) {
                for (int dx = 0; dx < 32 && (x + dx) < w; dx++) {
                    int px_idx = dy * 32 + dx;
                    unsigned char r = tile_data[px_idx];
                    unsigned char g = tile_data[1024 + px_idx];
                    unsigned char b = tile_data[2048 + px_idx];
                    dst[(y + dy) * w + (x + dx)].r = r;
                    dst[(y + dy) * w + (x + dx)].g = g;
                    dst[(y + dy) * w + (x + dx)].b = b;
                }
            }
        }
    }

    free(img->pixel_data.pixels);
    img->pixel_data.pixels = dst;

    free(tiles);
    free_dataset(ds);
}