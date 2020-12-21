.PHONY: build run profile

build:
	gcc -lm example.c -o example

run:
	gcc -lm example.c -o example
	./example

profile:
	gcc -pg -lm example.c -o example
	./example
	gprof example > profile.output
	cat profile.output