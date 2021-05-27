MKFILE      := $(lastword $(MAKEFILE_LIST))
WORKDIR     := libogg

include make/configure.make
BUILD_LIBOGG:= 1
include make/platform.make

TARGET	    := libogg_
SOURCES     := libs/libogg-1.3.4/src
INCLUDES    := 
LIBS 				:=

OGGOBJS     := src/bitwise.o \
               src/framing.o

Q3OBJ       := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OGGOBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS      := $(INCLUDE) -fsigned-char -MMD \
               -O2 -ftree-vectorize -ffast-math -fno-short-enums

define DO_OGG_CC
  @echo "OGG_CC $<"
  @$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) $(BR)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.c
	$(DO_OGG_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
