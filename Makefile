SHELL     := /bin/bash

TARGET     = map
objects    = $(patsubst %.c, %.o, $(wildcard src/*.c))
dependency = linenoise mbedtls mongoose kilo uptime uuid4 zlib

ifeq ($(PREFIX),)
  PREFIX  := /usr/local
endif

CFLAGS  += -Os -Wall $(foreach dep, $(dependency), $(if $(findstring $(dep), mbedtls), -I./deps/$(dep)/include, -I./deps/$(dep))) -I./include
LDFLAGS += $(foreach dep, $(dependency), $(if $(findstring $(dep), mbedtls zlib), , ./deps/$(dep)/$(dep).o)) \
	-L./deps/mbedtls/library -lmbedtls -lmbedcrypto -L./deps/zlib -lz -lm -pthread

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
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
ifneq ($(shell uname),Darwin)
	$(STRIP) --strip-all --remove-section=.comment $@
endif

.PHONY: deps
deps: $(dependency) mbedtls

.PHONY: $(dependency)
$(dependency):
	@if [[ $@ == mbedtls ]]; then                            \
		cd deps/$@ && CC=$(CC) $(MAKE) -j8 lib;            \
	elif [[ $@ == zlib ]]; then                              \
		cd deps/$@ && CC=$(CC) ./configure --static && CC=$(CC) make;   \
	else                                                     \
		cd deps/$@ && CC=$(CC) $(MAKE) -j8;                    \
	fi

.PHONY: clean-deps
clean-deps:
	@for dep in $(dependency); do                     \
		cd deps/$${dep} && $(MAKE) clean && cd ../..;   \
	done

.PHONY: clean
clean:
	@$(RM) map src/*.o *.out
	@$(MAKE) clean-deps

.PHONY: install
install:
	install -m 755 $(TARGET) $(PREFIX)/bin
