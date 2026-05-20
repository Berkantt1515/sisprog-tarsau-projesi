CC = gcc
CFLAGS = -Wall -Wextra -g

all: tarsau

tarsau: main.o tarsau_b.o
	$(CC) $(CFLAGS) -o tarsau main.o tarsau_b.o

main.o: main.c tarsau.h
	$(CC) $(CFLAGS) -c main.c

tarsau_b.o: tarsau_b.c tarsau.h
	$(CC) $(CFLAGS) -c tarsau_b.c

clean:
	rm -f *.o tarsau
