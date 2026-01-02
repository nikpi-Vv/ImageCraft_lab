#include "filter_tools.h"
#include <stdio.h>
#include "bmp_tools.h"

int parse_filters(int argc, char* argv[], int start, FilterNode** out_head) {
    *out_head = NULL;
    FilterNode* tail = NULL;

    for (int i = start; i < argc; i++) {
        const char* token = argv[i];
        if (token[0] != '-') {
            printf("Error: expected filter name starting with '-', got '%s'\n", token);
            free_filter_list(*out_head);
            return 0;
        }

        // Определяем имя без '-'
        const char* name = token + 1;
        int expected_params = 0;

        if (strcmp(name, "crop") == 0) expected_params = 2;
        else if (strcmp(name, "edge") == 0 || strcmp(name, "crystallize") == 0 ||
                strcmp(name, "glass") == 0 || strcmp(name, "blur") == 0 ||
                strcmp(name, "med") == 0) expected_params = 1;
        else if (strcmp(name, "gs") == 0 ||
                 strcmp(name, "neg") == 0 ||
                 strcmp(name, "sharp") == 0) {
            expected_params = 0;
        } else {
            printf("Unknown filter: %s\n", token);
            free_filter_list(*out_head);
            return 0;
        }

        if (i + expected_params >= argc) {
            printf("Error: filter '%s' requires %d parameter(s)\n", token, expected_params);
            free_filter_list(*out_head);
            return 0;
        }

        double* params = NULL;
        if (expected_params > 0) {
            params = malloc(expected_params * sizeof(double));
            if (!params) {
                printf("Out of memory\n");
                free_filter_list(*out_head);
                return 0;
            }

            for (int j = 0; j < expected_params; j++) {
                char* end;
                params[j] = strtod(argv[i + 1 + j], &end);
                if (*end != '\0') {
                    printf("Invalid parameter for filter '%s': '%s'\n", token, argv[i + 1 + j]);
                    free(params);
                    free_filter_list(*out_head);
                    return 0;
                }

                // === СЕМАНТИЧЕСКАЯ ВАЛИДАЦИЯ ===
                if (strcmp(name, "crop") == 0 || strcmp(name, "med") == 0) {
                    if (params[j] <= 0) {
                        printf("Error: '%s' requires positive integer parameter, got %g\n", token, params[j]);
                        free(params);
                        free_filter_list(*out_head);
                        return 0;
                    }
                } else if (strcmp(name, "blur") == 0 || strcmp(name, "glass") == 0) {
                    if (params[j] <= 0) {
                        fprintf(stderr, "Error: '%s' requires positive sigma/radius, got %g\n", token, params[j]);
                        free(params);
                        free_filter_list(*out_head);
                        return 0;
                    }
                } else if (strcmp(name, "edge") == 0) {
                    if (params[j] < 0 || params[j] > 1) {
                        fprintf(stderr, "Error: '-edge' threshold must be in [0, 1], got %g\n", params[j]);
                        free(params);
                        free_filter_list(*out_head);
                        return 0;
                    }
                }
            }
        }

        FilterNode* node = create_filter_node(name, params, expected_params);
        free(params); // данные скопированы

        if (!node) {
            printf("Out of memory\n");
            free_filter_list(*out_head);
            return 0;
        }

        if (!*out_head) {
            *out_head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }

        i += 1 + expected_params;
    }

    return 1;
}


char* string_duplicate(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}


int write_bmp_image(const char* output_path, BMP_Image* img) {
    if (!output_path || !img || !img->data) {
        fprintf(stderr, "Invalid arguments to write_bmp_image\n");
        return -1;
    }

    // Пересчитываем размер данных с учётом выравнивания
    int bytes_per_pixel = 3;
    int row_size = img->dib_header.width * bytes_per_pixel;
    int row_padding = (4 - (row_size % 4)) % 4;
    int padded_row_size = row_size + row_padding;
    unsigned int data_size = padded_row_size * img->dib_header.height;
    img->dib_header.raw_bitmap_data = data_size;
    img->bmp_header.size_of_file = sizeof(BMP_Header) + sizeof(DIB_Header) + data_size;

    FILE* f = fopen(output_path, "wb");
    if (!f) {
        perror("Failed to open output file");
        return -1;
    }

    // Записываем заголовки
    fwrite(&img->bmp_header, sizeof(BMP_Header), 1, f);
    fwrite(&img->dib_header, sizeof(DIB_Header), 1, f);

    // Записываем данные построчно с выравниванием
    unsigned char* padding = calloc(1, row_padding);
    if (!padding) {
        fclose(f);
        return -1;
    }

    unsigned char* row = malloc(padded_row_size);
    if (!row) {
        free(padding);
        fclose(f);
        return -1;
    }

    // BMP хранит строки **снизу вверх**
    for (int y = img->dib_header.height - 1; y >= 0; y--) {
        // Копируем строку из data
        memcpy(row, img->data + y * row_size, row_size);
        // Добавляем паддинг
        if (row_padding > 0) {
            memcpy(row + row_size, padding, row_padding);
        }
        fwrite(row, 1, padded_row_size, f);
    }

    free(row);
    free(padding);
    fclose(f);
    return 0;
}

// Создание нового узла фильтра
FilterNode* create_filter_node(const char* name, double* params, int param_count) {
    FilterNode* node = malloc(sizeof(FilterNode));
    if (!node) return NULL;

    node->name = string_duplicate(name);
    if (!node->name) {
        free(node);
        return NULL;
    }

    node->param_count = param_count;
    if (param_count > 0) {
        node->params = malloc(param_count * sizeof(double));
        if (!node->params) {
            free(node->name);
            free(node);
            return NULL;
        }
        memcpy(node->params, params, param_count * sizeof(double));
    } else {
        node->params = NULL;
    }

    node->next = NULL;
    return node;
}

// Освобождение списка фильтров
void free_filter_list(FilterNode* head) {
    while (head) {
        FilterNode* next = head->next;
        free(head->name);
        free(head->params);
        free(head);
        head = next;
    }
}

// Предварительное объявление функции применения фильтров
void apply_filters(BMP_Image* img, FilterNode* filters);