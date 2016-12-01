CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lncurses -pthread

all: pac pac_nouni

pac : pac.c

pac_nouni: pac_nouni.c

clean:
	rm -rf pac
