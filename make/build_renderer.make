REND_WORKDIR   := rend1
REND_SOURCE    := renderer

BUILD_RENDERER_OPENGL:=1
ifneq ($(BUILD_CLIENT),1)
MKFILE         := $(lastword $(MAKEFILE_LIST))
include make/platform.make
endif

REND_TARGET    := $(CNAME)_opengl1_$(SHLIBNAME)
ifeq ($(USE_MULTIVM_CLIENT),1)
REND_TARGET    := $(CNAME)_mw_opengl1_$(SHLIBNAME)
endif

WORKDIRS       += $(REND_WORKDIR)
CLEANS 	       += $(REND_WORKDIR) $(REND_TARGET)

NEED_COMMON_REND := 0
ifeq ($(USE_RENDERER_DLOPEN),1)
NEED_COMMON_REND := 1
endif
ifneq ($(BUILD_CLIENT),1)
REND_CFLAGS += -DUSE_RENDERER_DLOPEN
REND_CFLAGS += -DRENDERER_PREFIX=$(CNAME)
NEED_COMMON_REND := 1
endif


REND_SOURCES   := $(MOUNT_DIR)/$(REND_SOURCE) $(MOUNT_DIR)/renderercommon
REND_CFILES    := $(foreach dir,$(REND_SOURCES), $(wildcard $(dir)/*.c))

ifeq ($(NEED_COMMON_REND),1)
REND_CFILES    += $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
endif

REND_OBJS      := $(REND_CFILES:.c=.o) 
REND_Q3OBJ     := $(addprefix $(B)/$(REND_WORKDIR)/,$(notdir $(REND_OBJS)))

REND_CFLAGS    ?= $(BASE_CFLAGS) $(INCLUDE)
ifeq ($(BUILD_CLIENT),1)
REND_CFLAGS    += $(CLIENT_CFLAGS)
endif

define DO_REND_CC
  $(echo_cmd) "REND_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(REND_CFLAGS) -o $@ -c $<
endef

ifneq ($(BUILD_CLIENT),1)
debug:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 8 \
		REND_CFLAGS="$(REND_CFLAGS) $(DEBUG_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(REND_TARGET)

release:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 8 \
		REND_CFLAGS="$(REND_CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(REND_TARGET)

clean:
	@rm -rf ./$(BD)/$(REND_WORKDIR) ./$(BD)/$(REND_TARGET)
	@rm -rf ./$(BR)/$(REND_WORKDIR) ./$(BR)/$(REND_TARGET)
endif

ifdef B
$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/renderercommon/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/$(REND_SOURCE)/%.c
	$(DO_REND_CC)

$(B)/$(REND_TARGET):  $(addsuffix .mkdirs,$(addprefix $(B)/,$(WORKDIRS))) $(REND_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(REND_Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
