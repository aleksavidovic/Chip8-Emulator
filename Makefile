# Compiler and flags
CC = gcc
CFLAGS = -fopt-info-vec-all -Wall -Wextra -Iinclude -std=c11 
LIBS = -lSDL2

# Project structure
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGET = $(BUILDDIR)/chip8_emulator

# Find all .c source files in the src directory
SOURCES = $(wildcard $(SRCDIR)/*.c)
# Create a list of object files (.o) in the build directory
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

# --- Build Rules ---

# Default rule: build the final executable
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build successful! Executable is at $(TARGET)"

# Rule to compile source files into object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up build files
clean:
	@rm -rf $(BUILDDIR)
	@echo "Build directory cleaned."

