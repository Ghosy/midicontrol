# Compiler
CC := g++
# Compile-time flags
CFLAGS := -std=c++11 -Wall -g3

# Source directory
SRCDIR := src
# Build directory
BUILDDIR := build
# Bin directory
BINDIR := bin
# Exec name
TARGET := $(BINDIR)/midicontrol
# Documents directory
DOCSDIR := doc
# Tests directory
TESTDIR := tests

# Libraries linked to exec
LIBS = -lrtmidi -lboost_regex -lyaml-cpp

# Source extension
SRCEXT := cpp
# Source files
SRCS := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
# Object files
OBJS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.o))

$(TARGET): $(OBJS)
	@echo "Linking...";
	@mkdir -p $(BINDIR)
	@echo " $(CC) $^ -o $(TARGET) $(LIBS)"; $(CC) $^ -o $(TARGET) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) -c -o $@ $<"; $(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " Cleaning..."
	@echo " $(RM) -r $(BUILDDIR)"; $(RM) -r $(BUILDDIR)
	@echo " $(RM) -r $(BINDIR)"; $(RM) -r $(BINDIR)
	@echo " $(RM) -r $(DOCSDIR)/html"; $(RM) -r $(DOCSDIR)/html

docs:
	@doxygen $(DOCSDIR)/Doxyfile
	@xdg-open $(DOCSDIR)/html/index.html

# Test source files
SRCS_TEST := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
# *.o for midicontrol
OBJS_TEST := $(filter-out $(BUILDDIR)/keytest.o, $(OBJS))
# *.o for midicontrol tests
OBJS_TEST += $(patsubst $(TESTDIR)/%,$(BUILDDIR)/%,$(SRCS_TEST:.$(SRCEXT)=.o))

test: $(OBJS_TEST)
	@echo "Linking Test...";
	@mkdir -p $(BINDIR)
	@echo " $(CC) $(CFLAGS) $^ -o $(BINDIR)/$@ $(LIBS)"; $(CC) $(CFLAGS) $^ -o $(BINDIR)/$@ $(LIBS)

$(BUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@echo " $(CC) $(CFLAGS) -c -o $@ $<"; $(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

# DO NOT DELETE THIS LINE -- make depend needs it
