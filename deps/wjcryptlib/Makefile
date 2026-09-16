CFLAGS = -Os -Wall

sources = $(wildcard *.c)
objects = $(sources:.c=.o)

# 四种算法合成单个 wjcryptlib.o，保持 deps/<name>/<name>.o 的约定。
# -nostdlib：clang 在 -r 时仍会带上 libSystem，ld 会为此输出 dylib 警告。
wjcryptlib.o: $(objects)
	$(CC) -nostdlib -r -o $@ $(objects)

clean:
	@$(RM) -f *.o
