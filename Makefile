CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o

.PHONY: all
all: deps map

map: map.c lru.c hashmap.c command.c avl.c base64.c

.PHONY: deps
deps: linenoise murmurhash

linenoise:
	cd deps/$@ && $(MAKE)

murmurhash:
	cd deps/$@ && $(MAKE)

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-($(RM) map)
