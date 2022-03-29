BOT_WORKDIR      := botlib

BUILD_BOTLIB := 1
ifneq ($(BUILD_CLIENT),1)
MKFILE       := $(lastword $(MAKEFILE_LIST)) 
include make/platform.make
endif

BOT_TARGET       := $(CNAME)_libbots_$(SHLIBNAME)
ifeq ($(USE_MULTIVM_SERVER),1)
BOT_TARGET       := $(CNAME)_mw_libbots_$(SHLIBNAME)
endif
BOT_SOURCES      := $(MOUNT_DIR)/botlib
BOT_INCLUDES     := 
BOT_CFILES       := $(foreach dir,$(BOT_SOURCES), $(wildcard $(dir)/*.c)) \
                    $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c 
BOT_OBJS         := $(BOT_CFILES:.c=.o)
BOT_Q3OBJ        := $(addprefix $(B)/$(BOT_WORKDIR)/,$(notdir $(BOT_OBJS)))

export BOT_INCLUDE  := $(foreach dir,$(BOT_INCLUDES),-I$(dir))

BOT_CFLAGS  := $(BASE_CFLAGS) $(INCLUDE) -DUSE_BOTLIB_DLOPEN

define DO_BOTLIB_CC
  $(echo_cmd) "BOTLIB_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(BOT_CFLAGS) -DBOTLIB -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(BOT_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS=$(BOT_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 8 \
		BOT_CFLAGS="$(BOT_CFLAGS) $(DEBUG_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(BOT_TARGET)

release:
	$(echo_cmd) "MAKE $(BOT_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS=$(BOT_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 8 \
		BOT_CFLAGS="$(BOT_CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(BOT_TARGET)

clean:
	@rm -rf ./$(BD)/$(BOT_WORKDIR) ./$(BD)/$(BOT_TARGET)
	@rm -rf ./$(BR)/$(BOT_WORKDIR) ./$(BR)/$(BOT_TARGET)

ifdef B
$(B)/$(BOT_WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_BOTLIB_CC)

$(B)/$(BOT_WORKDIR)/%.o: $(MOUNT_DIR)/botlib/%.c
	$(DO_BOTLIB_CC)

$(B)/$(BOT_TARGET): $(BOT_Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(BOT_Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
