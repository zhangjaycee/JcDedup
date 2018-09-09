all: src/dedup.c
	gcc src/dedup.c src/utils.c -o jcdedup -lm -O3
clean:
	rm jcdedup
