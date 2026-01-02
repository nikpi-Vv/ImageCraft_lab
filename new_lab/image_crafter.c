// image_craft.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter_tools.h"
#include  "bmp_tools.h"

// Функция вывода справки
void print_usage(const char* program_name) {
    printf("Usage: %s <input.bmp> <output.bmp> [ -<filter> [params...] ]...\n", program_name);
    printf("Supported filters:\n");
    printf("  -crop <width> <height>\n");
    printf("  -gs\n");
    printf("  -neg\n");
    printf("  -sharp\n");
    printf("  -edge <threshold>\n");
    printf("  -med <window>\n");
    printf("  -blur <sigma>\n");
    printf("  -<custom1> ...\n");
    printf("  -<custom2> ...\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_path = argv[2];

    BMP_Image* img = NULL;
    // if (!validate_and_load_image(input_path, &img)) { // Функция под вопросом, может и надо, пользователь дебил
    //     return 1;
    // }

    FilterNode* filter_list = NULL;
    if (!parse_filters(argc, argv, 3, &filter_list)) {
        free_filter_list(filter_list);
        free_bmp_image(img);
        return 1;
    }

    apply_filters(img, filter_list);

    // TODO: save_output_image(output_path, img);

    free_filter_list(filter_list);
    free_bmp_image(img);
    return 0;
}


// int main(int argc, char *argv[]) {
//     if (argc < 3) {
//         print_usage(argv[0]);
//         return 1;
//     }

//     const char* input_image = argv[1];
//     const char* output_image = argv[2];

//     // Загрузка изображения
//     BMP_Image* img = parse_bmp_image(input_image);
//     if (!img) {
//         printf("Failed to load input image: %s\n", input_image);
//         return 1;
//     }

//     // Парсинг фильтров
//     FilterNode* filter_head = NULL;
//     FilterNode* filter_tail = NULL;

//     for (int i = 3; i < argc; ) {
//         // Определяем количество параметров по имени фильтра
//         int expected_params = 0;
//         if (strcmp(argv[i], "-crop") == 0) expected_params = 2;
//         else if (strcmp(argv[i], "-edge") == 0) expected_params = 1;
//         else if (strcmp(argv[i], "-med") == 0) expected_params = 1;
//         else if (strcmp(argv[i], "-blur") == 0) expected_params = 1;
//         else if (strcmp(argv[i], "-gs") == 0 ||
//                  strcmp(argv[i], "-neg") == 0 ||
//                  strcmp(argv[i], "-sharp") == 0) {
//             expected_params = 0;
//         } else {
//             printf("Unknown filter: %s\n", argv[i]);
//             free_bmp_image(img);
//             free_filter_list(filter_head);
//             return 1;
//         }

//         if (i + expected_params > argc) {
//             printf("Error: filter '%s' requires %d parameter(s)\n", argv[i], expected_params);
//             free_bmp_image(img);
//             free_filter_list(filter_head);
//             return 1;
//         }

//         double* params = NULL;
//         if (expected_params > 0) {
//             params = malloc(expected_params * sizeof(double));
//             if (!params) {
//                 printf("Out of memory\n");
//                 free_bmp_image(img);
//                 free_filter_list(filter_head);
//                 return 1;
//             }
//             for (int j = 0; j < expected_params; j++) {
//                 char* end;
//                 params[j] = strtod(argv[i + j], &end);
//                 if (*end != '\0') {
//                     printf("Invalid parameter for filter '%s': '%s'\n", argv[i], argv[i + j]);
//                     free(params);
//                     free_bmp_image(img);
//                     free_filter_list(filter_head);
//                     return 1;
//                 }
//             }
//         }

//         FilterNode* new_node = create_filter_node(argv[i], params, expected_params);
//         free(params); // данные скопированы внутрь узла
//         if (!new_node) {
//             printf("Out of memory\n");
//             free_bmp_image(img);
//             free_filter_list(filter_head);
//             return 1;
//         }

//         if (!filter_head) {
//             filter_head = filter_tail = new_node;
//         } else {
//             filter_tail->next = new_node;
//             filter_tail = new_node;
//         }

//         i += expected_params;
//     }

//     // Применение фильтров
//     apply_filters(img, filter_head);

    
//     // Очистка
//     free_filter_list(filter_head);
//     free_bmp_image(img);
//     return 0;
// }
