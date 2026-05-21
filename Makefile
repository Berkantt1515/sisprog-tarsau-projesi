CC = gcc
CFLAGS = -Wall

all: tarsau

tarsau: main.c tarsau_b.c
	$(CC) $(CFLAGS) main.c tarsau_b.c -o tarsau

clean:
	rm -f tarsau
