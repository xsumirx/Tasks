PREFIX ?= /usr/local

# Common Programs
RM        ?= rm

# The name of the output binary
TARGET    := sort

# Source files required to compile the target
SOURCES   := sort.c


OBJECTS   := $(patsubst %.c,%.o,$(SOURCES))


# Flags and options for the compiler/linker
CFLAGS    += -I.

# Default target
.PHONY: all clean run
all: $(TARGET)

# Target to remove build files
clean:
	$(RM) -r $(TARGET)
	$(RM) -r $(OBJECTS)
	

# Target to install the program
run: $(TARGET)
	./$(TARGET)

# Target to compile output binary from objects.
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Target to compile files into object code.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@