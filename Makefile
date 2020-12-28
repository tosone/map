CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash \
	-I./deps/mongoose -I./deps/kilo -I./deps/mbedtls/include -I./deps/uptime -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o \
	./deps/mongoose/mongoose.o ./deps/kilo/kilo.o ./deps/uptime/uptime.o \
	-L./deps/mbedtls/library -lmbedtls -lmbedcrypto -lmbedx509 -lm -pthread

objects := $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all
all: deps map

map: $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

.PHONY: deps
deps: linenoise murmurhash mbedtls mongoose kilo uptime

.PHONY: linenoise
linenoise:
	cd deps/$@ && $(MAKE)

.PHONY: murmurhash
murmurhash:
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

.PHONY: uptime
uptime:
	cd deps/$@ && $(MAKE)

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-(cd deps/mongoose && $(MAKE) clean) > /dev/null || true
	-(cd deps/mbedtls && $(MAKE) clean) > /dev/null || true
	-(cd deps/kilo && $(MAKE) clean) > /dev/null || true
	-(cd deps/uptime && $(MAKE) clean) > /dev/null || true
	-($(RM) map *.o)
