MKFILE         := $(lastword $(MAKEFILE_LIST))
WORKDIR        := rend1

BUILD_RENDERER_OPENGL:=1
include make/platform.make

TARGET         := $(RENDERER_PREFIX)_opengl1_$(SHLIBNAME)

SOURCES        := $(MOUNT_DIR)/renderer $(MOUNT_DIR)/renderercommon
INCLUDES       := 

CFILES         := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
                  $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
OBJS           := $(CFILES:.c=.o) 
Q3OBJ          := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS   := $(INCLUDE) -fsigned-char -ftree-vectorize \
            -ffast-math -fno-short-enums -MMD

define DO_REND_CC
  $(echo_cmd) "REND_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_REND_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/renderercommon/%.c
	$(DO_REND_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/renderer/%.c
	$(DO_REND_CC)

$(B)/$(TARGET): $(Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
