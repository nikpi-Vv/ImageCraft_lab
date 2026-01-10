#ifndef FILTER_TOOLS_H
#define FILTER_TOOLS_H

#include "bmp_tools.h"

// Структура для описания одного фильтра
typedef struct FilterNode {
    char* name;           // имя фильтра, например "crop"
    double* params;       // параметры фильтра (в double для удобства)
    int param_count;      // количество параметров
    struct FilterNode* next;
} FilterNode;

// Парсинг фильтров из команды
int parse_filters(int argc, char* argv[], int start, FilterNode** out_head);

// Создание нового узла фильтра
FilterNode* create_filter_node(const char* name, double* params, int param_count);

// Освобождение списка фильтров
void free_filter_list(FilterNode* head);

#endif