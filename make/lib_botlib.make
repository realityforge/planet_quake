MKFILE       := $(lastword $(MAKEFILE_LIST)) 
WORKDIR      := botlib

BUILD_BOTLIB := 1
include make/platform.make

TARGET       := $(BOTLIB_PREFIX)_libbots_$(SHLIBNAME)
ifeq ($(USE_MULTIVM_CLIENT),1)
TARGET       := $(BOTLIB_PREFIX)_libbots_mw_$(SHLIBNAME)
endif
ifeq ($(USE_MULTIVM_SERVER),1)
TARGET       := $(BOTLIB_PREFIX)_libbots_mw_$(SHLIBNAME)
endif

SOURCES      := $(MOUNT_DIR)/botlib
INCLUDES     := 
CFILES       := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
               $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c 
OBJS         := $(CFILES:.c=.o)
Q3OBJ        := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS  := $(INCLUDE) -fsigned-char -O2 -ftree-vectorize -g \
          -ffast-math -fno-short-enums -DUSE_BOTLIB_DLOPEN

define DO_BOTLIB_CC
  $(echo_cmd) "BOTLIB_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) -DBOTLIB -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_BOTLIB_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/botlib/%.c
	$(DO_BOTLIB_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
