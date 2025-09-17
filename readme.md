
  - How it works: Pass DIR=<subdir> to target a project folder containing main.c. If
  DIR is omitted, it targets ./main.c.
  - Build: make DIR=mutex builds mutex/main.
  - Run: make run DIR=mutex builds and runs mutex/main.
  - Clean: make clean DIR=mutex removes mutex/main and object files.
  - Output: Binary is created as <DIR>/main (or ./main if no DIR).
  - Validation: The Makefile errors if $(DIR)/main.c (or ./main.c) does not exist.


