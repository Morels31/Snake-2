all:
	gcc -I include/SDL3/ -l SDL3 -o snake src/snake.c

clean:
	rm -f snake
