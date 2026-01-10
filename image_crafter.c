#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter_tools.h"
#include  "bmp_tools.h"
#include "filters.h"

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
    printf("  -glass <radius>\n");
    printf("  -crystallize <cell size>\n");
    printf("  -unsharp_mask <amount>\n");
    printf("  -mosaic_cifar replace tiles with random CIFAR-100 images (requires 32x32)\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_path = argv[2];

    // Загрузка изображения
    BMP_Image* img = parse_bmp_image(input_path);
    if (!img) {
        printf("Failed to load input image: %s\n", input_path);
        return 1;
    }

    FilterNode* filter_list = NULL;
    if (!parse_filters(argc, argv, 3, &filter_list)) {
        free_filter_list(filter_list);
        free_bmp_image(img);
        return 1;
    }

    apply_filters(img, filter_list);

    if (!save_bmp_image(output_path, img)) {
        printf("Failed to save output image: %s\n", output_path);
        free_filter_list(filter_list);
        free_bmp_image(img);
        return 1;
    }

    free_filter_list(filter_list);
    free_bmp_image(img);

    printf("Successfully processed image: %s -> %s\n", input_path, output_path);
    return 0;
}
