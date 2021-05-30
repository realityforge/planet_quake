MKFILE           := $(lastword $(MAKEFILE_LIST))
WORKDIR          := ded

BUILD_SERVER     :=1
USE_DEDICATED    :=1
include make/platform.make

TARGET_SERVER    := $(DNAME)$(ARCHEXT)$(BINEXT)
ifeq ($(USE_MULTIVM_CLIENT),1)
TARGET_SERVER    := $(DNAME)_mw$(ARCHEXT)$(BINEXT)
endif

SOURCES  := $(MOUNT_DIR)/server $(MOUNT_DIR)/botlib

CLIPMAP  := $(B)/ded/cm_load.o \
            $(B)/ded/cm_load_bsp2.o \
            $(B)/ded/cm_patch.o \
            $(B)/ded/cm_polylib.o \
            $(B)/ded/cm_test.o \
            $(B)/ded/cm_trace.o

QCOMMON  := $(B)/ded/cmd.o \
            $(B)/ded/common.o \
            $(B)/ded/cvar.o \
            $(B)/ded/files.o \
            $(B)/ded/history.o \
            $(B)/ded/keys.o \
            $(B)/ded/md4.o \
            $(B)/ded/md5.o \
            $(B)/ded/msg.o \
            $(B)/ded/net_chan.o \
            $(B)/ded/net_ip.o \
            $(B)/ded/huffman.o \
            $(B)/ded/huffman_static.o \
            $(B)/ded/q_math.o \
            $(B)/ded/q_shared.o \
            $(B)/ded/unzip.o \
            $(B)/ded/puff.o

VM       := $(B)/ded/vm.o \
            $(B)/ded/vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
  VM     += $(B)/ded/vm_x86.o
endif
ifeq ($(ARCH),x86_64)
  VM     += $(B)/ded/vm_x86.o
endif
ifeq ($(ARCH),arm)
  VM     += $(B)/ded/vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
  VM     += $(B)/ded/vm_aarch64.o
endif
endif

CURL     :=
ifeq ($(USE_CURL),1)
  CURL   += $(B)/ded/cl_curl.o
endif


SYSTEM   :=
ifeq ($(PLATFORM),js)
  SYSTEM += $(B)/ded/sys_glimp.o \
            $(B)/ded/sys_main.o \
            $(B)/ded/sys_input.o \
            $(B)/ded/unix_shared.o

else
ifdef MINGW
  SYSTEM += $(B)/ded/win_main.o \
            $(B)/ded/win_shared.o \
            $(B)/ded/win_syscon.o \
            $(B)/ded/win_resource.o

else # !MINGW
  SYSTEM += $(B)/ded/unix_main.o \
            $(B)/ded/unix_shared.o \
            $(B)/ded/linux_signals.o

endif
endif


INCLUDES := $(MOUNT_DIR)/qcommon

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/sv_*.c)) \
            $(CLIPMAP) $(QCOMMON) $(VM) $(SYSTEM)
OBJS     := $(CFILES:.c=.o) 
Q3OBJ    := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS   := $(INCLUDE) -fsigned-char -ftree-vectorize \
            -ffast-math -fno-short-enums -MMD

# TODO build server as a library that client can launch many instances

define DO_SERVER_CC
  $(echo_cmd) "SERVER_CC $<"
  $(Q)$(CC) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(BD)/$(TARGET_SERVER)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET_SERVER)

release:
	$(echo_cmd) "MAKE $(BR)/$(TARGET_SERVER)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET_SERVER)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET_SERVER)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET_SERVER)

ifdef B
# TODO: cl_curl for server downloads, still need to fix sync lag
#$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/ded/%.c
#	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/unix/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/win32/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/macosx/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/wasm/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/server/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/botlib/%.c
	$(DO_SERVER_CC)

$(B)/$(TARGET_SERVER): $(Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(CLIENT_LDFLAGS) $(LDFLAGS) 
endif
