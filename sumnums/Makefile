INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O3 -fopenmp $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= sumnums.c ompsumnums.c 

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	sumnums ompsumnums 

clean:
	-rm -f *.o *.d
	-rm -f sumnums

distclean:
	-rm -f *.o *.d
	-rm -f sumnums

sumnums: sumnums.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

ompsumnums: ompsumnums.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread



depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
