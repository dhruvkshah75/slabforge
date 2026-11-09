CC ?= cc
AR ?= ar
CPPFLAGS += -D_DEFAULT_SOURCE -Iinclude -Isrc
CFLAGS += -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined
LDFLAGS += -fsanitize=address,undefined

BUILD_DIR := build
SOURCES := src/arena.c src/bump.c src/freelist.c src/mm.c
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
LIBRARY := $(BUILD_DIR)/libslabforge.a
TEST := $(BUILD_DIR)/test_basic
TEST_NAME ?= all
PYTHON ?= python3

.PHONY: all test run clean

all: $(LIBRARY)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c src/mm_internal.h include/mm.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

$(TEST): test/test_basic.c $(LIBRARY) include/mm.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) $(LDFLAGS) -o $@

test run:
	$(PYTHON) test/run_tests.py --test $(TEST_NAME)

clean:
	rm -rf $(BUILD_DIR)
