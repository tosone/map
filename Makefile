TARGET     = map
objects    = $(patsubst %.c, %.o, $(wildcard *.c))
dependency = linenoise murmurhash mbedtls mongoose kilo uptime uuid4 sds

ifeq ($(PREFIX),)
  PREFIX  := /usr/local
endif

CFLAGS  += -Os -Wall $(foreach dep, $(dependency), $(if $(findstring $(dep), mbedtls), -I./deps/$(dep)/include, -I./deps/$(dep))) -I./algo -I.
LDFLAGS += $(foreach dep, $(dependency), $(if $(findstring $(dep), mbedtls), , ./deps/$(dep)/$(dep).o)) \
	$(patsubst %.c, %.o, $(wildcard algo/*.c)) \
	-L./deps/mbedtls/library -lmbedtls -lmbedcrypto -lm -pthread

ifneq ($(shell uname),Darwin)
  CFLAGS  += -static
endif

.PHONY: all
all: $(TARGET) upx

.PHONY: upx
upx:
ifneq ($(shell uname),Darwin)
	upx --best $(TARGET)
endif

.PHONY: $(TARGET)
$(TARGET): $(objects)
	make -C algo
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: deps
deps: $(dependency)

.PHONY: $(dependency)
$(dependency):
	cd deps/$@ && $(MAKE)

.PHONY: clean-deps
clean-deps:
	@for dep in $(dependency); do                       \
		pushd deps/$${dep} && $(MAKE) clean && popd;      \
	done

.PHONY: clean
clean:
	@$(RM) map *.o algo/*.o
	@$(MAKE) clean-deps

.PHONY: install
install:
	install -m 755 $(TARGET) $(PREFIX)/bin
