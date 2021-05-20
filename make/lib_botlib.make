MKFILE   := $(lastword $(MAKEFILE_LIST)) 

include make/platform.make
include make/configure.make
include make/platform_os.make

LIB_PREFIX  := $(CNAME)
TARGET	    := $(LIB_PREFIX)_libbots_
SOURCES     := code/botlib
INCLUDES    := 

#LIBS = -l

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
	          code/qcommon/q_math.c code/qcommon/q_shared.c 
OBJS     := $(CFILES:.c=.o)
Q3OBJ    := $(addprefix $(B)/botlib/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

PREFIX  := 
CC      := gcc
CFLAGS  := $(INCLUDE) -fsigned-char \
          -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

SHLIBCFLAGS  := -fPIC -fno-common
SHLIBLDFLAGS := -dynamiclib $(LDFLAGS) \
							  -DUSE_BOTLIB_DLOPEN
SHLIBNAME    := $(ARCH).$(SHLIBEXT)

define DO_BOTLIB_CC
	@echo "BOTLIB_CC $<"
	@$(CC) $(SHLIBCFLAGS) $(CFLAGS) -DBOTLIB -o $@ -c $<
endef

mkdirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/botlib ];then $(MKDIR) $(B)/botlib;fi

default:
	$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

$(B)/botlib/%.o: code/qcommon/%.c
	$(DO_BOTLIB_CC)

$(B)/botlib/%.o: code/botlib/%.c
	$(DO_BOTLIB_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	@$(CC) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@

clean:
	@rm -rf $(B)/botlib


ifdef B
D_FILES=$(shell find $(BD)/botlib -name '*.d')
endif

ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean makedirs release \
  targets tools toolsclean mkdirs \
	$(D_FILES)

.DEFAULT_GOAL := default
