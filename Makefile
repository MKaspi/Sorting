
SHELL = /bin/bash
CC=gcc
CFLAGS=-static -std=gnu99 -Wall -pedantic -g

BUILD=build
SORTS_F=Sorts
SRC_F=Src

HEADERS=$(shell find . -type f -name '*.h')
SRC_C=$(shell find $(SRC_F) -type f -name '*.c')
SRC_O=$(patsubst %.c,%.o,$(SRC_C))

SORTS=$(patsubst $(SORTS_F)/%.c,%,$(wildcard $(SORTS_F)/*.c))

run: $(patsubst %,$(BUILD)/%.run,$(SORTS))
clean:
	rm -rf $(BUILD)

$(BUILD)/%.run: $(BUILD)/%.ex
	@mkdir -p $(@D)
	((time $< &> $(patsubst %.run,%.out,$@)) 2>$@.tmp && mv $@.tmp $@) &

$(BUILD)/%.ex: $(BUILD)/$(SORTS_F)/%.o $(patsubst %,$(BUILD)/%,$(SRC_O))
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(CFLAGS)

$(BUILD)/%.o: ./%.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) $< -c -o $@ $(CFLAGS)

.PRECIOUS: $(BUILD)/%.o $(BUILD)/%.ex $(BUILD)/%.run
