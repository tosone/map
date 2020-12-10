CFLAGS  += -I./deps/linenoise -I.
LDFLAGS += ./deps/linenoise/linenoise.o

.PHONY: all
all: deps map

map: map.c lru.c hashmap.c

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
	-(rm -f .make-*)
	-($(RM) map)
