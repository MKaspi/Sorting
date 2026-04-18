CC = gcc
SHELL = /bin/bash

CFLAGS = -O2 -Wall -Iinclude -fPIC
LDFLAGS = -ldl

# --- BUILD TARGETS ---

BUILD = .build
BIN_DIR = $(BUILD)/bin
PLUGIN_DIR = $(BUILD)/plugins

CORE_SRC = $(wildcard core/*.c)
BIN = bench

PLUGIN_SRC = $(wildcard plugins/*.c)
PLUGIN_SO = $(patsubst plugins/%.c,$(PLUGIN_DIR)/%.so,$(PLUGIN_SRC))

# --- DEFAULT ---

build: $(PLUGIN_SO) $(BIN)

# --- PLUGINS (.so) ---

$(PLUGIN_DIR)/%.so: plugins/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -shared $< -o $@

# --- CORE BINARY ---

$(BIN): $(CORE_SRC)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CORE_SRC) -o $@ $(LDFLAGS)

# --- TEST (example orchestration) ---

test: build
	@echo "Running example test (quick sort, 2048 elements)..."
	./$(BIN) $(PLUGIN_DIR)/quick.so \
	  --input generator \
	  --elem-count 2048 \
	  --elem-size 4

# --- CLEAN ---

clean:
	rm -rf $(BUILD) $(BIN)

