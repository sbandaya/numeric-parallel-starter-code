INCLUDE_DIRS = 
LIB_DIRS = 
#CC = icc
CC = gcc
CPP = mpic++

CDEFS=
CFLAGS= -O3 -g -Wall -fopenmp $(INCLUDE_DIRS) $(CDEFS)
CFLAGS2= -O0 -g -Wall -fopenmp $(INCLUDE_DIRS) $(CDEFS)
LIBS=

PRODUCT= hello_riemann hello_trapezoidal hello_simpson hello_rk4

HFILES= 
CFILES= 
HPPFILES= 
CPPFILES= hello_riemann.cpp hello_trapezoidal.cpp hello_simpson.cpp hello_rk4.cpp left_train.cpp rapezoidal_train.cpp mid_train.cpp

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.NEW *~
	-rm -f ${PRODUCT} ${DERIVED} ${GARBAGE}

hello_riemann:	hello_riemann.cpp
	$(CPP) $(CFLAGS2) -o $@ hello_riemann.cpp -lm

hello_trapezoidal:	hello_trapezoidal.cpp
	$(CPP) $(CFLAGS2) -o $@ hello_trapezoidal.cpp -lm

hello_simpson:	hello_simpson.cpp
	$(CPP) $(CFLAGS2) -o $@ hello_simpson.cpp -lm

hello_rk4:	hello_rk4.cpp
	$(CPP) $(CFLAGS2) -o $@ hello_rk4.cpp -lm

left_train:	left_train.cpp
	$(CPP) $(CFLAGS2) -o $@ left_train.cpp -lm

mid_train: mid_train.cpp
	$(CPP) $(CFLAGS2) -o $@ mid_train.cpp -lm

trapezoidal_train: trapezoidal_train.cpp
	$(CPP) $(CFLAGS2) -o $@ trapezoidal_train.cpp -lm
