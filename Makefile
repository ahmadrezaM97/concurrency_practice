CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS ?= -pthread

# Usage examples:
#   make run DIR=subdir   # builds and runs subdir/main
#   make list DIR=subdir  # shows sources used in subdir
#   make examples         # lists available example subdirectories

DIR ?=

# Discover example subdirectories (those that contain a main.c)
EXAMPLES := $(patsubst %/,%,$(sort $(dir $(wildcard */main.c))))

ifeq ($(strip $(DIR)),)

.PHONY: all run clean list examples

all:
	@echo "Please choose an example directory:" && \
	echo "  make run DIR=<one of: $(EXAMPLES)>" && \
	false

run: all

list: examples

examples:
	@echo "Available examples:" && \
	for d in $(EXAMPLES); do echo " - $$d"; done

clean:
	@for d in $(EXAMPLES); do $(MAKE) -s DIR=$$d clean || true; done

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

OBJS := $(SRCS:.c=.o)

.PHONY: all run clean list

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(DIR)/%.o: $(DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

list:
	@echo "Building: $(BIN)"
	@echo "Sources: $(SRCS)"

clean:
	rm -f $(BIN) $(OBJS)

endif
