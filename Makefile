# 1. Определение операционной системы
ifeq ($(OS),Windows_NT)
    RM = del /Q /F
    RMDIR = rmdir /S /Q
    MKDIR = if not exist int mkdir int
    EXT = .exe
    P = \\
    INT_DIR_CLEAN = int
else
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p int
    EXT =
    P = /
    INT_DIR_CLEAN = int/
endif

# 2. Настройки компилятора
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# 3. Названия исполняемых файлов
TARGET = image_craft$(EXT)
GEN_TARGET = gen_bmps$(EXT)

# 4. Поиск файлов
SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

# Исключаем файл с main() генератора из объектов основной программы
CORE_SOURCES = $(filter-out gen_bmps.c, $(SOURCES))
OBJECTS = $(patsubst %.c, int$(P)%.o, $(CORE_SOURCES))

# 5. Основные правила
all: int_dir $(TARGET) $(GEN_TARGET)

# Создание директории int
int_dir:
	@$(MKDIR)

# Сборка основной программы
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Универсальное правило для объектных файлов в папке int/
int$(P)%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка генератора
$(GEN_TARGET): gen_bmps.c
	$(CC) $(CFLAGS) -o $@ $<

# Запуск генератора
gen: $(GEN_TARGET)
	.$(P)$(GEN_TARGET)

# 6. Команды очистки
clean-int:
	$(RMDIR) $(INT_DIR_CLEAN)

clean: clean-int
	$(RM) $(TARGET) $(GEN_TARGET)

# 7. Справка
help:
	@echo "NAME"
	@echo "    $(TARGET) - image processing tool"
	@echo ""
	@echo "USAGE"
	@echo "    .$(P)$(TARGET) <input.bmp> <output.bmp> [ -<filter> [params...] ]..."
	@echo ""
	@echo "COMMANDS"
	@echo "    all         Build the main program and BMP generator"
	@echo "    gen         Run the BMP generator"
	@echo "    clean       Remove all build artifacts (including executable)"
	@echo "    clean-int   Remove only the intermediate $(INT_DIR_CLEAN) directory"
	@echo "    help        Display this help message"

.PHONY: all clean clean-int gen help int_dir
