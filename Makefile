SHELL     := /bin/bash

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

STRIP   := $(CROSS_COMPILE)strip

ifneq ($(shell uname),Darwin)
  CFLAGS += -static
  ifneq ($(ARCH),)
		TARGET := $(TARGET)-$(ARCH)
	endif
endif

.PHONY: all
all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET): $(objects)
	$(MAKE) -C algo
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
ifneq ($(shell uname),Darwin)
	$(STRIP) --strip-all --remove-section=.comment $@
endif

.PHONY: deps
deps: $(dependency) mbedtls

.PHONY: $(dependency)
$(dependency):
	@if [[ $@ == mbedtls ]]; then                   \
		cd deps/$@ && CC=$(CC) $(MAKE) -j8 no_test; \
	else                                            \
		cd deps/$@ && CC=$(CC) $(MAKE) -j8;         \
	fi

.PHONY: clean-deps
clean-deps:
	@for dep in $(dependency); do                       \
		cd deps/$${dep} && $(MAKE) clean && cd ../..;   \
	done

.PHONY: clean
clean:
	@$(RM) map *.o algo/*.o
	@$(MAKE) clean-deps

.PHONY: install
install:
	install -m 755 $(TARGET) $(PREFIX)/bin
