CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS ?= -pthread

# Usage examples:
#   make build DIR=subdir # builds subdir/main.c into subdir/main
#   make run DIR=subdir   # builds and runs subdir/main
#   make clean            # cleans all built files
#   make examples         # lists available example subdirectories

DIR ?=

# Discover example subdirectories (those that contain a main.c)
EXAMPLES := $(patsubst %/,%,$(sort $(dir $(wildcard */main.c))))

.PHONY: all build run clean examples

# Build command - simplified
build:
ifeq ($(strip $(DIR)),)
	@echo "Error: Please specify a directory"
	@echo "Usage: make build DIR=<directory>"
	@echo "Available examples: $(EXAMPLES)"
	@false
else
	@if [ ! -f "$(DIR)/main.c" ]; then \
		echo "Error: $(DIR)/main.c not found"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(wildcard $(DIR)/*.c) -o $(DIR)/main $(LDLIBS)
	@echo "Built: $(DIR)/main"
endif

run: build
	./$(DIR)/main

all:
	@echo "Available examples: $(EXAMPLES)"
	@echo "Usage: make build DIR=<directory>"

examples:
	@echo "Available examples:"
	@for d in $(EXAMPLES); do echo " - $$d"; done

clean:
	@find . -type f -name 'main' -path '*/main' -delete
	@find . -type f -name '*.o' -delete
	@echo "Cleaned all binaries and object files"
