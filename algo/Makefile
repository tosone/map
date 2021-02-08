CFLAGS   += -Os -Wall -I../deps/murmurhash -I.
LDFLAGS  += ../deps/murmurhash/murmurhash.o -lm -pthread

objects = $(patsubst %.c, %.o, $(wildcard *.c))

all: $(objects)
