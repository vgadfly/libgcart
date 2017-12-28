CFLAGS=$(shell pkg-config --cflags gobject-2.0)
LDFLAGS=$(shell pkg-config --libs gobject-2.0)
CC=gcc

CFLAGS+=-O0 -g

all: bin

bin: tl-parse tl-gen test

tl-parse: lex.yy.c tl.tab.c parse.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tl-gen: lex.yy.c tl.tab.c gen.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

lex.yy.c: tl.tab.c tl.l
	flex tl.l

tl.tab.c: tl.y
	bison -t -d --report all tl.y

test: test.c dummy.c object.c auto.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

auto.c: tl-gen
	./tl-gen test.tl

.PHONY: clean

clean:
	rm tl.tab.c tl.tab.h lex.yy.c auto.h auto.c auto/*

