all: src/dedup.c
	gcc src/dedup.c -o jcdedup -lm
clean:
	rm jcdedup
