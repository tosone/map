CFLAGS = -Os -Wall

sources = $(wildcard *.c)
objects = $(sources:.c=.o)

# Merge the four algorithms into a single wjcryptlib.o to keep the
# deps/<name>/<name>.o convention used by the main Makefile.
# -nostdlib: clang still pulls in libSystem when linking with -r, which makes
# ld print a dylib warning.
wjcryptlib.o: $(objects)
	$(CC) -nostdlib -r -o $@ $(objects)

clean:
	@$(RM) -f *.o
