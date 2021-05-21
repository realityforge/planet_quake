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

mkdirs:
  @if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
  @if [ ! -d $(B) ];then $(MKDIR) $(B);fi
  @if [ ! -d $(B)/$(WORKDIR) ];then $(MKDIR) $(B)/$(WORKDIR);fi

default:
  @echo "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
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

D_FILES := $(@shell find $(BD)/$(WORKDIR) -name '*.d') \
					 $(@shell find $(BR)/$(WORKDIR) -name '*.d')
ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif

.PHONY: $(.PHONY) \
	$(D_FILES)

.DEFAULT_GOAL := default
