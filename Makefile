CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude
SRC = src/main.c src/color.c src/image.c src/bmp.c \
      src/filters_basic.c src/filters_extra.c src/pipeline.c 
//потом будет менять в засимости от файлов
OBJ = $(SRC:.c=.o)

image_craft: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(OBJ) image_craft
