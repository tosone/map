CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash \
	-I./deps/mongoose -I./deps/kilo -I./deps/mbedtls/include \
	-I./deps/libtomcrypt/src/headers -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o \
	./deps/mongoose/mongoose.o ./deps/kilo/kilo.o \
	-L./deps/libtomcrypt -L./deps/mbedtls/library -ltomcrypt -lmbedtls \
	-lmbedcrypto -lmbedx509 -lm -pthread

objects := $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all
all: deps map

map: $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

.PHONY: deps
deps: linenoise murmurhash libtomcrypt mongoose mbedtls kilo

.PHONY: linenoise
linenoise:
	cd deps/$@ && $(MAKE)

.PHONY: murmurhash
murmurhash:
	cd deps/$@ && $(MAKE)

.PHONY: libtomcrypt
libtomcrypt:
	cd deps/$@ && $(MAKE)

.PHONY: mongoose
mongoose:
	cd deps/$@ && $(MAKE)

.PHONY: mbedtls
mbedtls:
	cd deps/$@ && $(MAKE) programs

.PHONY: kilo
kilo:
	cd deps/$@ && $(MAKE)

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-(cd deps/libtomcrypt && $(MAKE) clean) > /dev/null || true
	-(cd deps/mongoose && $(MAKE) clean) > /dev/null || true
	-(cd deps/mbedtls && $(MAKE) clean) > /dev/null || true
	-(cd deps/kilo && $(MAKE) clean) > /dev/null || true
	-($(RM) map *.o)
