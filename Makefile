SHELL      := /bin/bash

TARGET     = map
objects    = $(patsubst %.c, %.o, $(wildcard src/*.c))
dependency = linenoise mongoose kilo uptime uuid4 wjcryptlib

ifeq ($(PREFIX),)
  PREFIX  := /usr/local
endif

CFLAGS  += -Os -Wall $(foreach dep, $(dependency), -I./deps/$(dep)) -I./include
LDFLAGS += $(foreach dep, $(dependency), ./deps/$(dep)/$(dep).o) -lm -pthread

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
deps: $(dependency)

.PHONY: $(dependency)
$(dependency):
	@cd deps/$@ && CC=$(CC) $(MAKE) -j8

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
