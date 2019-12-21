all: first

first: first.c 
	gcc -std=c11 -Wall -Werror -fsanitize=address  first.c -o first -lm

clean:
	rm -rf first
