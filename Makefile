CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o

ifneq ($(shell uname),Darwin)
  CFLAGS += -static
endif

.PHONY: all
all: clean deps map upx

.PHONY: upx
upx:
ifneq ($(shell uname),Darwin)
	upx -9 map
endif

map: map.c lru.c hashmap.c command.c avl.c base64.c

.PHONY: deps
deps: linenoise murmurhash

.PHONY: linenoise
linenoise:
	cd deps/$@ && $(MAKE)

.PHONY: murmurhash
murmurhash:
	cd deps/$@ && $(MAKE)

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-($(RM) map)
