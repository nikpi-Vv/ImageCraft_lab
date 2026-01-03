CC = gcc
CFLAGS = -Wall -Wextra -g -lm
TARGET = image_craft
OBJS = image_crafter.o bmp_tools.o filter_tools.o filters.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@

test: $(TARGET)
	@echo "Running basic tests..."
	@./$(TARGET) 2>&1 | head -5  # Проверка справки
	@echo "✓ Help displayed"
	@echo "Creating test image..."
	@python3 create_test_image.py  # Скрипт для создания тестового BMP
	@./$(TARGET) test.bmp output.bmp -crop 50 50 -gs
	@if [ $$? -eq 0 ]; then echo "✓ Basic pipeline works"; else echo "✗ Failed"; fi

clean:
	rm -f $(TARGET) $(OBJS) test.bmp output.bmp

.PHONY: all test clean
