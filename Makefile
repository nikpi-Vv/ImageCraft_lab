CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = image_crafter.exe
SOURCES = bmp_tools.c filter_tools.c filters.c image_crafter.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = bmp_tools.h filter_tools.h filters.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(OBJECTS) $(TARGET) 2>nul || exit 0

test: $(TARGET)
	@echo Компиляция завершена. Запускайте:
	@echo $(TARGET) input.bmp output.bmp [фильтры]
	@echo Пример: $(TARGET) test.bmp result.bmp -crop 50 50 -gs -neg

.PHONY: all clean test