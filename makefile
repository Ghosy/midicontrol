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
# bin file for midicontrol tests
TARGET_TEST := $(BINDIR)/test

test: $(TARGET_TEST)

$(TARGET_TEST): $(OBJS_TEST)
	@echo "Linking Test...";
	@mkdir -p $(BINDIR)
	@echo " $(CC) $(CFLAGS) $^ -o $@ $(LIBS)"; $(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(BUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@echo " $(CC) $(CFLAGS) -c -o $@ $<"; $(CC) $(CFLAGS) -c -o $@ $<

PREFIX := /usr/local

BIN_INSTALL := $(DESTDIR)$(PREFIX)/bin/midicontrol
MAN_INSTALL := $(DESTDIR)$(PREFIX)/share/man/man1/midicontrol.1.gz

install: $(BIN_INSTALL) $(MAN_INSTALL)

$(BIN_INSTALL):
	install -Dm 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

$(MAN_INSTALL):
	install -Dm 644 doc/midicontrol.1 $(DESTDIR)$(PREFIX)/share/man/man1/
	gzip $(DESTDIR)$(PREFIX)/share/man/man1/midicontrol.1

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/midicontrol
	rm $(DESTDIR)$(PREFIX)/share/man/man1/midicontrol.1.gz

.PHONY: clean docs test install uninstall

# DO NOT DELETE THIS LINE -- make depend needs it
