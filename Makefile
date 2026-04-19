CC = gcc
SHELL = /bin/bash

CFLAGS = -O2 -Wall -Iinclude -fPIC
LDFLAGS = -ldl

# --- BUILD TARGETS ---

BUILD = .build
BIN_DIR = $(BUILD)/bin
PLUGIN_DIR = $(BUILD)/plugins
TEST_DIR = $(BUILD)/tests

CORE_SRC = $(wildcard core/*.c)
BIN = bench

PLUGIN_SRC = $(wildcard plugins/*.c)
HEADERS = $(wildcard include/*.h)
PLUGIN_SO = $(patsubst plugins/%.c,$(PLUGIN_DIR)/%.so,$(PLUGIN_SRC))

TESTS = $(patsubst plugins/%.c,$(TEST_DIR)/%.out,$(PLUGIN_SRC))

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

# --- TEST (example orchestration) ---

test: $(TESTS)

$(TEST_DIR)/%.out: $(PLUGIN_DIR)/%.so $(BIN)
	mkdir -p $(@D)
	./$(BIN) $< \
	  --dataset internal \
	  --elem-count 200 \
	  --print-steps \
	  > $@

# --- CLEAN ---

clean:
	rm -rf $(BUILD) $(BIN)

