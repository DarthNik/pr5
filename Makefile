CC = gcc
CFLAGS = -Wall

all: signal

signal: signal.c
	$(CC) $(CFLAGS) signal.c -o signal

clean:
	rm -rf signal
