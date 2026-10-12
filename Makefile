TARGET := bin/8086
SRC := src/main.c
CFLAGS := -Wall -Werror -Wextra -Wpedantic -O0 -g -fsanitize=address -lasan

default:
	gcc $(SRC) -o $(TARGET) $(CFLAGS)

.PHONY: run debug

run: default
	./$(TARGET)

debug: default
	raddbg ./$(TARGET)
