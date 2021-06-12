REND_WORKDIR   := rendes
REND_SOURCE    := rendereres

BUILD_RENDERER_OPENGLES:=1
ifneq ($(BUILD_CLIENT),1)
MKFILE         := $(lastword $(MAKEFILE_LIST))
include make/platform.make
endif

REND_TARGET    := $(RENDERER_PREFIX)_opengles_$(SHLIBNAME)
REND_SOURCES   := $(MOUNT_DIR)/$(REND_SOURCE) $(MOUNT_DIR)/$(REND_SOURCE)/glsl $(MOUNT_DIR)/renderercommon
CFILES         := $(foreach dir,$(REND_SOURCES), $(wildcard $(dir)/*.c))
ifeq ($(USE_RENDERER_DLOPEN),1)
CFILES         := $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
endif
REND_OBJS      := $(CFILES:.c=.o) 
Q3R2STRINGOBJ  := $(GLSLFILES:.glsl=.o)
REND_Q3OBJ     := $(addprefix $(B)/$(REND_WORKDIR)/,$(notdir $(REND_OBJS)))

CFLAGS         ?= $(INCLUDE) -fsigned-char -ftree-vectorize \
                  -ffast-math -fno-short-enums -MMD
ifneq ($(BUILD_CLIENT),1)
CFLAGS         += $(SHLIBCFLAGS)
endif

define DO_REND_CC
  $(echo_cmd) "REND_CC $<"
  $(Q)$(CC) $(CFLAGS) -o $@ -c $<
endef

ifneq ($(BUILD_CLIENT),1)
debug:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) REND_WORKDIR=$(REND_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(REND_TARGET)

release:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) REND_WORKDIR=$(REND_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(REND_TARGET)

clean:
	@rm -rf $(BD)/$(REND_WORKDIR) $(BD)/$(REND_TARGET)
	@rm -rf $(BR)/$(REND_WORKDIR) $(BR)/$(REND_TARGET)
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
