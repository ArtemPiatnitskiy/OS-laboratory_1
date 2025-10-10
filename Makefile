CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I include

HEADERS = include/string_to_lowercase.h include/space_replace.h

# Входные файлы и цели
TARGET = program
CHILD1 = child1
CHILD2 = child2

# Основное правило: собираем основной бинарник и детей
all: $(TARGET) $(CHILD1) $(CHILD2)

# main слинковывается с общими модулями
$(TARGET): src/main.o src/string_to_lowercase.o src/space_replace.o
	$(CC) $(CFLAGS) -o $@ $^

# Правило компиляции main.c
main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

# Универсальное правило для исходников в src
src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка child1 как отдельного исполняемого файла
$(CHILD1): src/child1.c src/string_to_lowercase.c
	$(CC) $(CFLAGS) -o $(CHILD1) src/child1.c src/string_to_lowercase.c

# Сборка child2 как отдельного исполняемого файла
$(CHILD2): src/child2.c src/space_replace.c
	$(CC) $(CFLAGS) -o $(CHILD2) src/child2.c src/space_replace.c

# Очистка
clean:
	rm -f main.o src/*.o $(TARGET) $(CHILD1) $(CHILD2)

.PHONY: all clean
