all: src/dedup.c
	gcc src/dedup.c -o jcdedup -lm -g
clean:
	rm jcdedup
