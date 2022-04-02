MKFILE           := $(lastword $(MAKEFILE_LIST))
WORKDIR          := ded

BUILD_SERVER     := 1
USE_DEDICATED    := 1
include make/platform.make

TARGET_SERVER    := $(DNAME)$(ARCHEXT)$(BINEXT)
ifeq ($(USE_MULTIVM_SERVER),1)
TARGET_SERVER    := $(DNAME)_mw$(ARCHEXT)$(BINEXT)
endif
ifeq ($(BUILD_EXPERIMENTAL),1)
TARGET_SERVER    := $(DNAME)_experimental$(ARCHEXT)$(BINEXT)
endif

ifeq ($(USE_MEMORY_MAPS),1)
ifeq ($(USE_INTERNAL_JPEG),1)
BUILD_CLIENT     := 1
include make/lib_jpeg.make
endif
endif

SOURCES  := $(MOUNT_DIR)/server $(MOUNT_DIR)/botlib

CLIPMAP  := cm_load.o cm_load_bsp2.o \
            cm_patch.o cm_polylib.o \
            cm_test.o cm_trace.o

QCOMMON  := cmd.o cmd_descriptions.o common.o \
            cvar.o cvar_descriptions.o \
            files.o files_async.o history.o keys.o \
            md4.o md5.o msg.o \
            net_chan.o net_ip.o huffman.o \
            huffman_static.o q_math.o q_shared.o \
            unzip.o puff.o

VM       := vm.o \
            vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
  VM     += vm_x86.o
endif
ifeq ($(ARCH),x86_64)
  VM     += vm_x86.o
endif
ifeq ($(ARCH),arm)
  VM     += vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
  VM     += vm_aarch64.o
endif
endif

CURL     :=
ifeq ($(USE_CURL),1)
  CURL   += cl_curl.o
endif


SYSTEM   :=
ifeq ($(PLATFORM),js)
  SYSTEM += sys_glimp.o sys_main.o \
            sys_input.o unix_shared.o

else
ifdef MINGW
  SYSTEM += win_main.o win_shared.o \
            win_syscon.o win_resource.o

else # !MINGW
  SYSTEM += unix_main.o unix_shared.o \
            linux_signals.o

endif
endif


SERVER_INCLUDES  := $(MOUNT_DIR)/qcommon

SERVER_CFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/sv_*.c)) \
            $(CLIPMAP) $(QCOMMON) $(VM) $(SYSTEM)
ifneq ($(USE_BOTLIB_DLOPEN),1)
SERVER_CFILES    += $(foreach dir,$(SOURCES), $(wildcard $(dir)/be_*.c)) \
            $(foreach dir,$(SOURCES), $(wildcard $(dir)/l_*.c))
endif

ifeq ($(USE_MEMORY_MAPS),1)
SERVER_LDFLAGS   += $(BR)/$(CNAME)_q3map2_$(SHLIBNAME)
SERVER_CFILES    += cl_jpeg.o
endif

SERVER_OBJS      := $(SERVER_CFILES:.c=.o) 
SERVER_Q3OBJ     := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(SERVER_OBJS)))

ifeq ($(USE_MEMORY_MAPS),1)
ifeq ($(USE_INTERNAL_JPEG),1)
SERVER_Q3OBJ            += $(JPEGOBJS)
endif
endif

export INCLUDE  := $(foreach dir,$(SERVER_INCLUDES),-I$(dir))

SERVER_CFLAGS := $(BASE_CFLAGS) $(INCLUDE)

ifeq ($(USE_MEMORY_MAPS),1)
ifeq ($(USE_SYSTEM_JPEG),1)
BASE_CFLAGS      += $(JPEG_CFLAGS)
SERVER_LDFLAGS   += $(JPEG_LIBS)
endif
endif

# TODO build server as a library that client can launch many instances

define DO_SERVER_CC
  $(echo_cmd) "SERVER_CC $<"
  $(Q)$(CC) -o $@ $(SERVER_CFLAGS) -c $<
endef

ifdef WINDRES
define DO_WINDRES
	$(echo_cmd) "WINDRES $<"
	$(Q)$(WINDRES) -o $@ -i $<
endef
endif

define DO_BOT_CC
	$(echo_cmd) "BOT_CC $<"
	$(Q)$(CC) -o $@ $(SERVER_CFLAGS) -DBOTLIB -c $<
endef

debug:
	$(echo_cmd) "MAKE $(BD)/$(TARGET_SERVER)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 8 \
    SERVER_CFLAGS="$(SERVER_CFLAGS) $(DEBUG_CFLAGS)" \
    SERVER_LDFLAGS="$(SERVER_LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET_SERVER)

release:
	$(echo_cmd) "MAKE $(BR)/$(TARGET_SERVER)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 8 \
    SERVER_CFLAGS="$(SERVER_CFLAGS) $(RELEASE_CFLAGS)" \
    SERVER_LDFLAGS="$(SERVER_LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET_SERVER)

clean:
	@rm -rf ./$(BD)/$(WORKDIR) ./$(BD)/$(TARGET_SERVER)
	@rm -rf ./$(BR)/$(WORKDIR) ./$(BR)/$(TARGET_SERVER)

ifdef B

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/unix/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/win32/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/win32/%.rc
	$(DO_WINDRES)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/macosx/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/wasm/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_SERVER_CC)

# server might use cl_curl or cl_jpeg
$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/client/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/server/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/botlib/%.c
	$(DO_BOT_CC)

$(B)/$(TARGET_SERVER): $(addsuffix .mkdirs,$(addprefix $(B)/,$(WORKDIRS))) $(SERVER_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(SERVER_Q3OBJ) $(CLIENT_LDFLAGS) $(SERVER_LDFLAGS)
endif
