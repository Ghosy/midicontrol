# Compiler
CC := g++
# Compile-time flags
CFLAGS := -std=c++11 -Wall -g3

# Source directory
SRCDIR := src
# Build directory
BUILDDIR := build
# Exec name
TARGET := bin/midicontrol
# Documents directory
DOCSDIR := doc

# Libraries linked to exec
LIBS = -lrtmidi -lboost_regex

# Source extension
SRCEXT := cpp
# Source files
SRCS := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
# Object files
OBJS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.o))

$(TARGET): $(OBJS)
	@echo "Linking...";
	@echo " $(cc) $^ -o $(TARGET) $(LIBS)"; $(CC) $^ -o $(TARGET) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) -c -o $@ $<"; $(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " Cleaning..."
	@echo " $(RM) -r $(BUILDDIR)"; $(RM) -r $(BUILDDIR)
	@echo " $(RM) $(TARGET)"; $(RM) $(TARGET)
	@echo " $(RM) -r $(DOCSDIR)/html"; $(RM) -r $(DOCSDIR)/html

docs:
	@doxygen $(DOCSDIR)/Doxyfile
	@xdg-open $(DOCSDIR)/html/index.html

.PHONY: clean

# DO NOT DELETE THIS LINE -- make depend needs it