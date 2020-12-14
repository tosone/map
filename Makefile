CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash -I./deps/libtomcrypt/src/headers -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o ./deps/libtomcrypt/libtomcrypt.a

source := $(wildcard *.c)

ifneq ($(shell uname),Darwin)
  CFLAGS += -static
endif

.PHONY: all
all: deps map upx

.PHONY: upx
upx:
ifneq ($(shell uname),Darwin)
	upx -9 map
endif

map: $(source)

.PHONY: deps
deps: linenoise murmurhash libtomcrypt

.PHONY: linenoise
linenoise:
	cd deps/$@ && $(MAKE)

.PHONY: murmurhash
murmurhash:
	cd deps/$@ && $(MAKE)

.PHONY: libtomcrypt
libtomcrypt:
	cd deps/$@ && $(MAKE)

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-(cd deps/libtomcrypt && $(MAKE) clean) > /dev/null || true
	-($(RM) map)
