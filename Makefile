TARGET := bin/8086
SRC := src/8086.c src/parse.c
CFLAGS := -std=c23 -Wall -Werror -Wextra -Wpedantic -O0 -g -fsanitize=address

ifeq ($(shell uname -s),Linux)
	CFLAGS += -lasan
endif

default:
	gcc $(SRC) -o $(TARGET) $(CFLAGS)

.PHONY: run debug

run: default
	./$(TARGET)

debug: default
	raddbg ./$(TARGET)
