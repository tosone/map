SHELL      := /bin/bash

TARGET     = map
cobjects   = $(patsubst %.c, %.o, $(wildcard src/*.c))
cxxobjects = $(patsubst %.cpp, %.o, $(wildcard src/*.cpp))
objects    = $(cobjects) $(cxxobjects)
dependency = linenoise mongoose kilo uptime uuid4

CXX       ?= c++

ifeq ($(PREFIX),)
  PREFIX  := /usr/local
endif

CFLAGS   += -Os -Wall $(foreach dep, $(dependency), -I./deps/$(dep)) -I./include
CXXFLAGS += $(CFLAGS) -std=c++17
LDFLAGS  += $(foreach dep, $(dependency), ./deps/$(dep)/$(dep).o) -lm -pthread

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
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
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
