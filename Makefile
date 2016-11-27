CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lncurses -pthread

all: pac

clean:
	rm -rf pac
