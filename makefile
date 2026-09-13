###############################################################################
#                                                                             #
# MAKEFILE for clk                                                            #
#                                                                             #
# (c) Guy Wilson 2026                                                         #
#                                                                             #
###############################################################################

# Version number for clk
MAJOR_VERSION = 1
MINOR_VERSION = 0

# Directories
SOURCE = src
BUILD = build
DEP = dep
DOC = doc

MANSOURCE = $(DOC)/clk.1.md

# What is our target
TARGET = clk
MANTARGET = $(TARGET).1

# Tools
VBUILD = vbuild
C = gcc
CPP = g++
LINKER = g++
PANDOC = pandoc

# postcompile step
PRECOMPILE = @ mkdir -p $(BUILD) $(DEP)
# postcompile step
POSTCOMPILE = @ mv -f $(DEP)/$*.Td $(DEP)/$*.d

COMMON_FLAGS=-c -Wall -pedantic
DEP_FLAGS=-MT $@ -MMD -MP -MF $(DEP)/$*.Td

CFLAGS_BASE=$(COMMON_FLAGS)
CFLAGS_REL=$(CFLAGS_BASE) -O2 $(DEP_FLAGS)
CFLAGS_DBG=$(CFLAGS_BASE) -g -O0 $(DEP_FLAGS)

CPPFLAGS_BASE=$(COMMON_FLAGS) -std=c++20
CPPFLAGS_REL=$(CPPFLAGS_BASE) -O2 $(DEP_FLAGS)
CPPFLAGS_DBG=$(CPPFLAGS_BASE) -g -O0 $(DEP_FLAGS)

ifndef DEBUG
CPPFLAGS=$(CPPFLAGS_REL)
CFLAGS=$(CFLAGS_REL)
else
CPPFLAGS=$(CPPFLAGS_DBG)
CFLAGS=$(CFLAGS_DBG)
endif

PANDOCFLAGS = -s -t man

# Libraries
STDLIBS=
EXTLIBS=-lgcrypt -lpng

COMPILE.cpp=$(CPP) $(CPPFLAGS) -o $@
COMPILE.c=$(C) $(CFLAGS) -o $@
LINK.o=$(LINKER) -o $@

PANDOC.md = $(PANDOC) $(PANDOCFLAGS) -o $@

CSRCFILES = $(wildcard $(SOURCE)/*.c)
CPPSRCFILES = $(wildcard $(SOURCE)/*.cpp)
OBJFILES = $(patsubst $(SOURCE)/%.c, $(BUILD)/%.o, $(CSRCFILES)) $(patsubst $(SOURCE)/%.cpp, $(BUILD)/%.o, $(CPPSRCFILES))
DEPFILES = $(patsubst $(SOURCE)/%.c, $(DEP)/%.d, $(CSRCFILES)) $(patsubst $(SOURCE)/%.cpp, $(DEP)/%.d, $(CPPSRCFILES))

all: $(TARGET) $(MANTARGET)

# Compile C/C++ source files
#
$(TARGET): $(OBJFILES)
	$(LINK.o) $^ $(EXTLIBS)

$(BUILD)/%.o: $(SOURCE)/%.c
$(BUILD)/%.o: $(SOURCE)/%.c $(DEP)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(BUILD)/%.o: $(SOURCE)/%.cpp
$(BUILD)/%.o: $(SOURCE)/%.cpp $(DEP)/%.d
	$(PRECOMPILE)
	$(COMPILE.cpp) $<
	$(POSTCOMPILE)

$(MANTARGET): $(MANSOURCE)
	$(PANDOC.md) $<

.PRECIOUS = $(DEP)/%.d
$(DEP)/%.d: ;

-include $(DEPFILES)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin
	cp $(TARGET).1 /usr/local/share/man/man1

version:
	$(VBUILD) -i $(TARGET).ver -t version.c.template -o $(SOURCE)/version.c -major $(MAJOR_VERSION) -minor $(MINOR_VERSION)

clean:
	rm -r $(BUILD)
	rm -r $(DEP)
	rm $(TARGET)
	rm $(TARGET).1
