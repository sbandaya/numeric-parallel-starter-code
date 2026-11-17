INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O3 -g -fopenmp $(INCLUDE_DIRS) $(CDEFS)

LIBS= -lm

HFILES= 
CFILES= sequential_segmented_sieve.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	sequential_segmented_sieve

clean:
	-rm -f *.o *.d
	-rm -f sequential_segmented_sieve

sequential_segmented_sieve: sequential_segmented_sieve.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIB_DIRS) $(LIBS)


depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $<
