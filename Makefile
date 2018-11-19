CC=clang
LD=clang
CFLAGS+=-Wall -Wextra -O3 -I./libbele -g
LDFLAGS+=-g
TARG=uodemodat

OFILES=\
	main.o\
	decode.o\
	gost.o\

HFILES=\
	dat.h\
	fns.h\

all: dep $(TARG)

$(TARG): $(OFILES) $(HFILES)
	$(LD) $(LDFLAGS) -o $(TARG) $(OFILES) libbele/libbele.a

%.$.o: %.c
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f *.o

nuke: clean
	rm -f $(TARG)

libbele:
	git clone -q https://github.com/0intro/libbele

dep: libbele
	cd libbele; $(MAKE)

cleandep:
	rm -rf libbele
