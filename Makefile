default:
	gcc src/main.c -o bin/8086 -Iinclude/ -Wall -Werror -Wextra -Wpedantic -O0 -g -fsanitize=address -lasan

.PHONY: run

run: default
	./bin/8086
