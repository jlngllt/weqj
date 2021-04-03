.POSIX:
CC      = cc
CFLAGS  = -ansi -pedantic -Wall -Wextra -Wno-missing-field-initializers -O2 -g
LDFLAGS =
LDLIBS  =
PREFIX  = /usr/local

all: weqj

weqj: weqj.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ weqj.c $(LDLIBS)

clean:
	rm -f weqj
