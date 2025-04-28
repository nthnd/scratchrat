CC = clang
CFLAGS = -std=c23 -pedantic -W -Wall -Wextra -O2
LDFLAGS = -lm -lpthread -ldl -lrt -lX11 -lraylib

all: scratchrat
scratchrat: scratchrat.o
scratchrat.o: scratchrat.c

clean:
	rm -rf *.o

.PHONY: all clean
