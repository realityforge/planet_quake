MKFILE   := $(lastword $(MAKEFILE_LIST))
WORKDIR  := libogg

include make/configure.make
include make/platform.make

TARGET	    := libogg
SOURCES     := libs/libogg-1.3.4/src
INCLUDES    := 
LIBS 				:=

OGGOBJS  := src/bitwise.o \
						src/framing.o

Q3OBJ    := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OGGOBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS  := $(INCLUDE) -fsigned-char -MMD \
           -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

define DO_OGG_CC
	@echo "OGG_CC $<"
	@$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

default:
  $(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.c
	$(DO_OGG_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@
endif
