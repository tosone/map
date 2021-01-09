CFLAGS  += -Os -Wall -I./deps/linenoise -I./deps/murmurhash \
	-I./deps/mongoose -I./deps/kilo -I./deps/mbedtls/include \
	-I./deps/uptime -I./deps/uuid4 -I.
LDFLAGS += ./deps/linenoise/linenoise.o ./deps/murmurhash/murmurhash.o \
	./deps/mongoose/mongoose.o ./deps/kilo/kilo.o ./deps/uptime/uptime.o  \
	./deps/uuid4/uuid4.o -L./deps/mbedtls/library -lmbedtls -lmbedcrypto -lm -pthread

objects := $(patsubst %.c,%.o,$(wildcard *.c))

ifneq ($(shell uname),Darwin)
  CFLAGS += -static
endif

.PHONY: all
all: map upx

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
deps: linenoise murmurhash mbedtls mongoose kilo uptime uuid4

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

.PHONY: uuid4
uuid4:
	cd deps/$@ && $(MAKE)

.PHONY: clean
clean:
	-(cd deps/linenoise && $(MAKE) clean) > /dev/null || true
	-(cd deps/murmurhash && $(MAKE) clean) > /dev/null || true
	-(cd deps/mongoose && $(MAKE) clean) > /dev/null || true
	-(cd deps/mbedtls && $(MAKE) clean) > /dev/null || true
	-(cd deps/kilo && $(MAKE) clean) > /dev/null || true
	-(cd deps/uptime && $(MAKE) clean) > /dev/null || true
	-(cd deps/uuid4 && $(MAKE) clean) > /dev/null || true
	-($(RM) map *.o)
