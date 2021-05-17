MKFILE=$(lastword $(MAKEFILE_LIST)) 

include make/platform.make
include make/configure.make

TARGET		:= botlib

SOURCES  := code/botlib
INCLUDES := code/botlib

#LIBS = -l

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
	code/qcommon/q_math.c code/qcommon/q_shared.c 
CPPFILES   := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
Q3OBJ    := $(addprefix $(B)/botlib/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

PREFIX  = 
CC      = gcc
CFLAGS  = $(INCLUDE) -fsigned-char \
        -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

SHLIBEXT=dylib
SHLIBCFLAGS=-fPIC -fno-common
SHLIBLDFLAGS=-dynamiclib $(LDFLAGS)

define DO_BOTLIB_CC
	@echo "BOTLIB_CC $<"
	@$(CC) $(SHLIBCFLAGS) $(CFLAGS) -DBOTLIB -DUSE_BOTLIB_DLOPEN -o $@ -c $<
endef

mkdirs:
	@if [ ! -d $(B)/botlib ];then $(MKDIR) $(B)/botlib;fi

default:
	$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(TARGET).$(SHLIBEXT)

$(B)/botlib/%.o: code/qcommon/%.c
	$(DO_BOTLIB_CC)

$(B)/botlib/%.o: code/botlib/%.c
	$(DO_BOTLIB_CC)

$(TARGET).$(SHLIBEXT): $(Q3OBJ) 
	@echo "LD $@"
	@$(CC) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@

clean:
	@rm -rf $(B)/botlib

.PHONY: default
.DEFAULT_GOAL := default
