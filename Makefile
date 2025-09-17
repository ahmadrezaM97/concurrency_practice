CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS ?= -pthread

# Usage examples:
#   make                 # builds ./main from main.c + helpers
#   make run             # builds and runs ./main
#   make DIR=subdir      # builds subdir/main from subdir/*.c
#   make run DIR=subdir  # builds and runs subdir/main

DIR ?=

# Determine sources and binary based on DIR
ifeq ($(strip $(DIR)),)
SRC_MAIN := main.c
BIN := main

ifeq ($(wildcard $(SRC_MAIN)),)
$(error Source file '$(SRC_MAIN)' not found. Create it or set DIR=<subdir>.)
endif

# All C sources at repo root
ALL_SRCS := $(wildcard *.c)
# Any C sources that define their own main (exclude main.c itself)
MAIN_SRCS := $(filter-out $(SRC_MAIN),$(shell grep -l -E '^[[:space:]]*int[[:space:]]+main\b' $(ALL_SRCS) 2>/dev/null))
# Extra sources without main
EXTRA_SRCS := $(filter-out $(SRC_MAIN) $(MAIN_SRCS),$(ALL_SRCS))
SRCS := $(SRC_MAIN) $(EXTRA_SRCS)
else
SRC_MAIN := $(DIR)/main.c
BIN := $(DIR)/main

ifeq ($(wildcard $(SRC_MAIN)),)
$(error Source file '$(SRC_MAIN)' not found. Set DIR=<valid-subdir>.)
endif

# All C sources in subdir
ALL_SRCS := $(wildcard $(DIR)/*.c)
MAIN_SRCS := $(filter-out $(SRC_MAIN),$(shell grep -l -E '^[[:space:]]*int[[:space:]]+main\b' $(ALL_SRCS) 2>/dev/null))
EXTRA_SRCS := $(filter-out $(SRC_MAIN) $(MAIN_SRCS),$(ALL_SRCS))
SRCS := $(SRC_MAIN) $(EXTRA_SRCS)
endif

OBJS := $(SRCS:.c=.o)

.PHONY: all run clean list

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

list:
	@echo "Building: $(BIN)"
	@echo "Sources: $(SRCS)"

clean:
	rm -f $(BIN) $(OBJS)
