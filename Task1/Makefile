PREFIX ?= /usr/local

# Common Programs
INSTALL   ?= install
RM        ?= rm

# The name of the output binary
TARGET    := gpio

# Source files required to compile the target
SOURCES   := gpio.c\
			gpio_backend_dummy.c\
			gpio_backend_sysfs.c


OBJECTS   := $(patsubst %.c,%.o,$(SOURCES))


# Flags and options for the compiler/linker
CFLAGS    += -I.
LIBS      += -pthread

# Default target
.PHONY: all clean install
all: $(TARGET)

# Target to remove build files
clean:
	$(RM) -r $(TARGET)
	$(RM) -r $(OBJECTS)
	

# Target to install the program
install: $(TARGET)
	$(INSTALL) -D $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	
# Target to uninstall the program.
uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

# Target to compile output binary from objects.
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Target to compile files into object code.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@