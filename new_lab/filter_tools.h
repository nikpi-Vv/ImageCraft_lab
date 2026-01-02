// filter_tools.h

#ifndef FILTER_TOOLS_H
#define FILTER_TOOLS_H

// Структура для описания одного фильтра
typedef struct FilterNode {
    char* name;           // имя фильтра, например "crop"
    double* params;       // параметры фильтра (в double для удобства)
    int param_count;      // количество параметров
    struct FilterNode* next;
} FilterNode;

int parse_filters(int argc, char* argv[], int start, FilterNode** out_head);

FilterNode* create_filter_node(const char* name, double* params, int param_count);

void free_filter_list(FilterNode* head);

void apply_filters(BMP_Image* img, FilterNode* filters);

 
#endif