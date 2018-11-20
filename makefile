# Compiler
CXX := g++
# Compile-time flags
CXXFLAGS := -std=c++14 -pedantic -Wall -Wextra -g3

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
TESTDIR := test

# Libraries linked to exec
LIBS = -lrtmidi -lboost_regex -lyaml-cpp -lpthread

# Source extension
SRCEXT := cpp
# Source files
SRCS := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
# Object files
OBJS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.o))

# Set PREFIX if not set
ifeq ($(strip $(PREFIX)),)
	PREFIX := /usr/local
endif

$(TARGET): $(OBJS)
	@echo "Linking...";
	@mkdir -p $(BINDIR)
	@echo " $(CXX) $^ -o $(TARGET) $(LIBS)"; $(CXX) $^ -o $(TARGET) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " CXX    $@"; $(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@echo "Cleaning..."
	@echo " RM    $(BUILDDIR)"; $(RM) -r $(BUILDDIR)
	@echo " RM    $(BINDIR)"; $(RM) -r $(BINDIR)
	@echo " RM    $(DOCSDIR)/html"; $(RM) -r $(DOCSDIR)/html

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
	@echo " $(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)"; $(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(BUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@echo " CXX    $@"; $(CXX) $(CXXFLAGS) -c -o $@ $<


BIN_INSTALL := $(DESTDIR)$(PREFIX)/bin/midicontrol
MAN_INSTALL := $(DESTDIR)$(PREFIX)/share/man/man1/midicontrol.1.gz
BSH_COMP_INSTALL := /usr/share/bash-completion/completions/midicontrol

install: $(BIN_INSTALL) $(MAN_INSTALL) $(BSH_COMP_INSTALL)

$(BIN_INSTALL):
	install -Dm 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

$(MAN_INSTALL):
	install -Dm 644 doc/midicontrol.1 $(DESTDIR)$(PREFIX)/share/man/man1/
	gzip $(DESTDIR)$(PREFIX)/share/man/man1/midicontrol.1

$(BSH_COMP_INSTALL):
	install -Dm 644 doc/midicontrol.bashcomp $@

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/midicontrol
	rm $(DESTDIR)$(PREFIX)/share/man/man1/midicontrol.1.gz
	rm $(BSH_COMP_INSTALL)

.PHONY: clean docs test install uninstall

# DO NOT DELETE THIS LINE -- make depend needs it
