CFLAGS += -I./deps/linenoise
LDFLAGS += ./deps/linenoise/linenoise.o

.PHONY: all
all: deps map

map: map.c

.PHONY: deps
deps: linenoise

linenoise:
	cd deps/$@ && $(MAKE)

.PHONY: distclean
distclean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(rm -f .make-*)
