CFLAGS=$(shell pkg-config --cflags glib-2.0)
LDFLAGS=$(shell pkg-config --libs glib-2.0)
CC=gcc

tl-parse: lex.yy.c tl.tab.c parse.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

lex.yy.c: tl.tab.c tl.l
	flex tl.l

tl.tab.c: tl.y
	bison -t -d --report all tl.y

clean:
	rm tl.tab.c tl.tab.h lex.yy.c

