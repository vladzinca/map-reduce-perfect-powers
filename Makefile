CC = gcc
CFLAGS = -lpthread -lm

build: tema1.c
	$(CC) -o tema1 tema1.c $(CFLAGS)

.PHONY : clean
clean :
	rm -f tema1
