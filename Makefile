CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash \
	-I./deps/mongoose \
	-I./deps/libtomcrypt/src/headers -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o \
	./deps/mongoose/mongoose.o \
	-L./deps/libtomcrypt -ltomcrypt -lm -pthread

objects := $(patsubst %.c,%.o,$(wildcard *.c))

ifneq ($(shell uname),Darwin)
  CFLAGS += -static
endif

.PHONY: all
all: deps map upx

.PHONY: upx
upx:
ifneq ($(shell uname),Darwin)
	upx --best map
endif

map: $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

.PHONY: deps
deps: linenoise murmurhash libtomcrypt mongoose mbedtls

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

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-(cd deps/libtomcrypt && $(MAKE) clean) > /dev/null || true
	-(cd deps/mongoose && $(MAKE) clean) > /dev/null || true
	-(cd deps/mbedtls && $(MAKE) clean) > /dev/null || true
	-($(RM) map *.o)
