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

#$(TEST_DIR)/%.out: $(PLUGIN_DIR)/%.so $(BIN)
#	mkdir -p $(@D)
#	./$(BIN) $< \
#	  --dataset internal \
#	  --elem-count 200 \
#	  --print-steps \
#	  > $@

$(TEST_DIR)/%.out: $(PLUGIN_DIR)/%.so $(BIN)
	mkdir -p $(@D)
	cp input/d1.int .build/tmp.int
	/bin/time -f " \n \
	  === Výkon programu === \n \
	  Reálný čas (wall clock): %E \n \
	  Uživatelský čas (CPU):   %U \n \
	  Systémový čas:           %S \n \
	  Využití CPU:             %P \n \
	  Max. paměť (KB):         %M \n \
	  Prům. paměť (KB):        %K \n \
	  Počet I/O vstupů:        %I \n \
	  Počet I/O výstupů:       %O \n \
	  Počet context switchů:   %c \n \
	  " ./$(BIN) $< \
	      --dataset .build/tmp.int \
	      &> $@

# --- CLEAN ---

clean:
	rm -rf $(BUILD) $(BIN)

