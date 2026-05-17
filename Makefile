CC = gcc
SHELL = /bin/bash

CFLAGS = -O2 -Wall -Iinclude -fPIC
LDFLAGS = -ldl

# --- BUILD TARGETS ---

BUILD = .build
BIN_DIR = $(BUILD)/bin
PLUGIN_DIR = $(BUILD)/plugins

CORE_SRC = $(wildcard core/*.c)
BIN = benchmarker

PLUGIN_SRC = $(wildcard plugins/*.c)
HEADERS = $(wildcard include/*.h)
PLUGIN_SO = $(patsubst plugins/%.c,$(PLUGIN_DIR)/%.so,$(PLUGIN_SRC))

# --- DEFAULT ---

build: $(PLUGIN_SO) $(BIN)

# --- PLUGINS (.so) ---

$(PLUGIN_DIR)/%.so: plugins/%.c $(HEADERS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -shared $< -o $@

# --- CORE BINARY ---

$(BIN): $(CORE_SRC) $(HEADERS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CORE_SRC) -o $@ $(LDFLAGS) -rdynamic

clean:
	rm -rf $(BUILD) $(BIN)


include Makefile.bench.mf

