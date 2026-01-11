#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* простая запись 24-битного BMP width x height одним цветом (R,G,B) */
int write_solid_bmp(const char* filename, int width, int height,
                    unsigned char r, unsigned char g, unsigned char b)
{
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("gen_bmps: cannot open '%s'\n", filename);
        return 0;
    }

    int bytes_per_pixel = 3;
    int row_size        = width * bytes_per_pixel;
    int row_padding     = (4 - (row_size % 4)) % 4;
    int raw_data_size   = (row_size + row_padding) * height;
    int file_size       = 54 + raw_data_size;

    unsigned char header[54] = {0};

    // BMP header
    header[0] = 'B';
    header[1] = 'M';
    *(unsigned int*)&header[2]  = file_size;
    *(unsigned int*)&header[10] = 54;

    // DIB header (BITMAPINFOHEADER)
    *(unsigned int*)&header[14] = 40;
    *(int*)&header[18]          = width;
    *(int*)&header[22]          = height;
    *(unsigned short*)&header[26] = 1;
    *(unsigned short*)&header[28] = 24;
    *(unsigned int*)&header[30]   = 0;
    *(unsigned int*)&header[34]   = raw_data_size;
    *(int*)&header[38]            = 2835;
    *(int*)&header[42]            = 2835;

    fwrite(header, 1, 54, f);

    unsigned char* row = malloc(row_size + row_padding);
    if (!row) {
        fclose(f);
        return 0;
    }
    for (int x = 0; x < width; x++) {
        row[x*3 + 0] = b;
        row[x*3 + 1] = g;
        row[x*3 + 2] = r;
    }
    for (int p = 0; p < row_padding; p++)
        row[row_size + p] = 0;

    for (int y = 0; y < height; y++)
        fwrite(row, 1, row_size + row_padding, f);

    free(row);
    fclose(f);
    printf("gen_bmps: created %s (%dx%d)\n", filename, width, height);
    return 1;
}

int main(void)
{
    // маленькие и специальные картинки
    write_solid_bmp("small32.bmp", 32, 32,   255, 0, 0);   // красный
    write_solid_bmp("small64.bmp", 64, 64,   0, 255, 0);   // зелёный
    write_solid_bmp("tiny.bmp",     2,  2,   0, 0, 255);   // синий
    write_solid_bmp("wide.bmp",   4096, 1,  128, 128, 128); // очень широкое
    write_solid_bmp("tall.bmp",      1, 4096, 200, 200, 0); // очень высокое
    write_solid_bmp("oddpad.bmp",    3,    3,  50, 100, 150); // паддинг 3 байта

    // плохие по формату тесты делаем просто переименованием
    // notbmp.bmp: создаём текстовый файл
    FILE* t = fopen("notbmp.bmp", "w");
    if (t) {
        fputs("THIS IS NOT A BMP\n", t);
        fclose(t);
        printf("gen_bmps: created notbmp.bmp (fake bmp)\n");
    }

    return 0;
}
