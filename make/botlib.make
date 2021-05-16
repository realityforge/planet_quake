TARGET		:= botlib
GIT_VERSION := $(shell git describe --abbrev=6 --dirty --always --tags)
MKDIR=mkdir
MKFILE=$(lastword $(MAKEFILE_LIST)) 

SOURCES  := code/botlib
INCLUDES := code/botlib

#LIBS = -l

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
	code/qcommon/q_math.c code/qcommon/q_shared.c 
CPPFILES   := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
Q3OBJ    := $(addprefix build/botlib/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

PREFIX  = 
CC      = gcc
CXX      = g++
CFLAGS  = $(INCLUDE) -fsigned-char \
        -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

SHLIBEXT=dylib
SHLIBCFLAGS=-fPIC -fno-common
SHLIBLDFLAGS=-dynamiclib $(LDFLAGS)

define DO_BOTLIB_CC
	@echo "BOTLIB_CC $<"
	@$(CC) $(SHLIBCFLAGS) $(CFLAGS) -DBOTLIB -DUSE_BOTLIB_DLOPEN -o $@ -c $<
endef

mkdirs:
	@if [ ! -d build/botlib ];then $(MKDIR) build/botlib;fi

default:
	$(MAKE) -f $(MKFILE) mkdirs
	$(MAKE) -f $(MKFILE) "$(TARGET).$(SHLIBEXT)"

build/botlib/%.o: code/qcommon/%.c
	$(DO_BOTLIB_CC)

build/botlib/%.o: code/botlib/%.c
	$(DO_BOTLIB_CC)

$(TARGET).$(SHLIBEXT): $(Q3OBJ) 
	@echo "LD $@"
	@$(CC) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@

clean:
	@rm -rf build/botlib

.PHONY: default
.DEFAULT_GOAL := default
