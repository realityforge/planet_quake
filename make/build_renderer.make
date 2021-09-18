REND_WORKDIR   := rend1
REND_SOURCE    := renderer

BUILD_RENDERER_OPENGL:=1
ifneq ($(BUILD_CLIENT),1)
MKFILE         := $(lastword $(MAKEFILE_LIST))
include make/platform.make
endif

REND_TARGET    := $(RENDERER_PREFIX)_opengl1_$(SHLIBNAME)
REND_SOURCES   := $(MOUNT_DIR)/$(REND_SOURCE) $(MOUNT_DIR)/renderercommon
REND_CFILES    := $(foreach dir,$(REND_SOURCES), $(wildcard $(dir)/*.c))
ifeq ($(USE_RENDERER_DLOPEN),1)
REND_CFILES    := $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
endif
REND_OBJS      := $(REND_CFILES:.c=.o) 
REND_Q3OBJ     := $(addprefix $(B)/$(REND_WORKDIR)/,$(notdir $(REND_OBJS)))

REND_CFLAGS    ?= $(INCLUDE) -fsigned-char -ftree-vectorize \
                  -ffast-math -fno-short-enums -MMD

define DO_REND_CC
  $(echo_cmd) "REND_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

ifneq ($(BUILD_CLIENT),1)
debug:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS=$(REND_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(REND_CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS=$(REND_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(REND_CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf ./$(BD)/$(REND_WORKDIR) ./$(BD)/$(REND_TARGET)
	@rm -rf ./$(BR)/$(REND_WORKDIR) ./$(BR)/$(REND_TARGET)
else
WORKDIRS += $(REND_WORKDIR)
CLEANS 	 += $(REND_WORKDIR) $(REND_TARGET)
endif

ifdef B
$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/renderercommon/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/$(REND_SOURCE)/%.c
	$(DO_REND_CC)

$(B)/$(REND_TARGET): $(REND_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(REND_Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
