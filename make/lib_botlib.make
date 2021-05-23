MKFILE   := $(lastword $(MAKEFILE_LIST)) 

include make/configure.make
BUILD_BOTLIB=1
include make/platform.make

LIB_PREFIX  := $(CNAME)
TARGET	    := $(LIB_PREFIX)_libbots_
SOURCES     := code/botlib
INCLUDES    := 
CFILES      := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
	             code/qcommon/q_math.c code/qcommon/q_shared.c 
OBJS        := $(CFILES:.c=.o)
Q3OBJ       := $(addprefix $(B)/botlib/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS  := $(INCLUDE) -fsigned-char -O2 -ftree-vectorize -g \
          -ffast-math -fno-short-enums -DUSE_BOTLIB_DLOPEN

define DO_BOTLIB_CC
	@echo "BOTLIB_CC $<"
	$(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) -DBOTLIB -o $@ -c $<
endef

default:
	$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=botlib mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/botlib/%.o: code/qcommon/%.c
	$(DO_BOTLIB_CC)

$(B)/botlib/%.o: code/botlib/%.c
	$(DO_BOTLIB_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif

clean:
	@rm -rf $(BD)/botlib $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/botlib $(BR)/$(TARGET)$(SHLIBNAME)
