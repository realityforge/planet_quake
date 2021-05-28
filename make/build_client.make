MKFILE      := $(lastword $(MAKEFILE_LIST)) 

BUILD_CLIENT:=1
include make/platform.make

LIB_PREFIX       := $(CNAME)
TARGET_CLIENT    := $(CNAME)$(ARCHEXT)$(BINEXT)

SOURCES  := $(MOUNT_DIR)/client

CLIPMAP  := \
            $(B)/client/cm_load.o \
            $(B)/client/cm_load_bsp2.o \
            $(B)/client/cm_patch.o \
            $(B)/client/cm_polylib.o \
            $(B)/client/cm_test.o \
            $(B)/client/cm_trace.o

QCOMMON  := \
            $(B)/client/cmd.o \
            $(B)/client/common.o \
            $(B)/client/cvar.o \
            $(B)/client/files.o \
            $(B)/client/history.o \
            $(B)/client/keys.o \
            $(B)/client/md4.o \
            $(B)/client/md5.o \
            $(B)/client/msg.o \
            $(B)/client/net_chan.o \
            $(B)/client/net_ip.o \
            $(B)/client/qrcodegen.o \
            $(B)/client/huffman.o \
            $(B)/client/huffman_static.o \
            $(B)/client/q_math.o \
            $(B)/client/q_shared.o \
            $(B)/client/unzip.o \
            $(B)/client/puff.o \
            $(B)/client/sv_init.o \
            $(B)/client/sv_main.o \
            $(B)/client/sv_bot.o \
            $(B)/client/sv_game.o

SOUND    := \
            $(B)/client/snd_adpcm.o \
            $(B)/client/snd_dma.o \
            $(B)/client/snd_mem.o \
            $(B)/client/snd_mix.o \
            $(B)/client/snd_wavelet.o \
            \
            $(B)/client/snd_main.o \
            $(B)/client/snd_codec.o \
            $(B)/client/snd_codec_wav.o \
            $(B)/client/snd_codec_ogg.o \
            $(B)/client/snd_codec_opus.o

ifeq ($(ARCH),x86)
ifndef MINGW
  SOUND  += \
            $(B)/client/snd_mix_mmx.o \
            $(B)/client/snd_mix_sse.o
endif
endif
  VM     := \
            $(B)/client/vm.o \
            $(B)/client/vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
  VM     += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),x86_64)
  VM     += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),arm)
  VM     += $(B)/client/vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
  VM     += $(B)/client/vm_aarch64.o
endif
endif

CURL     :=
ifeq ($(USE_CURL),1)
  CURL   += $(B)/client/cl_curl.o
endif


SYSTEM   :=
ifeq ($(PLATFORM),js)
  SYSTEM += \
            $(B)/client/sys_glimp.o \
            $(B)/client/sys_main.o \
            $(B)/client/sys_input.o \
            $(B)/client/unix_shared.o

else
ifdef MINGW
  SYSTEM += \
            $(B)/client/win_main.o \
            $(B)/client/win_shared.o \
            $(B)/client/win_syscon.o \
            $(B)/client/win_resource.o

ifeq ($(USE_SDL),1)
  SYSTEM += \
            $(B)/client/sdl_glimp.o \
            $(B)/client/sdl_gamma.o \
            $(B)/client/sdl_input.o \
            $(B)/client/sdl_snd.o

else # !USE_SDL
  SYSTEM += \
            $(B)/client/win_gamma.o \
            $(B)/client/win_glimp.o \
            $(B)/client/win_input.o \
            $(B)/client/win_minimize.o \
            $(B)/client/win_qgl.o \
            $(B)/client/win_snd.o \
            $(B)/client/win_wndproc.o
ifeq ($(USE_VULKAN_API),1)
  SYSTEM += \
            $(B)/client/win_qvk.o
endif
endif # !USE_SDL

else # !MINGW
  SYSTEM += \
            $(B)/client/unix_main.o \
            $(B)/client/unix_shared.o \
            $(B)/client/linux_signals.o

ifeq ($(USE_SDL),1)
  SYSTEM += \
            $(B)/client/sdl_glimp.o \
            $(B)/client/sdl_gamma.o \
            $(B)/client/sdl_input.o \
            $(B)/client/sdl_snd.o
else # !USE_SDL
  SYSTEM += \
            $(B)/client/linux_glimp.o \
            $(B)/client/linux_qgl.o \
            $(B)/client/linux_snd.o \
            $(B)/client/x11_dga.o \
            $(B)/client/x11_randr.o \
            $(B)/client/x11_vidmode.o
endif

ifeq ($(USE_VULKAN_API),1)
  SYSTEM += \
            $(B)/client/linux_qvk.o
endif
endif # !USE_SDL
endif


INCLUDES := $(MOUNT_DIR)/qcommon
CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/cl_*.c)) \
            $(CLIPMAP) \
            $(QCOMMON) \
            $(SOUND) \
            $(VM) \
            $(CURL) \
            $(SYSTEM)
OBJS     := $(CFILES:.c=.o) 
Q3OBJ    := $(addprefix $(B)/client/,$(notdir $(OBJS)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS   := $(INCLUDE) -fsigned-char -ftree-vectorize \
            -ffast-math -fno-short-enums -MMD

ifdef BUILD_SLIM_CLIENT
CFLAGS   += -DBUILD_SLIM_CLIENT
endif

# TODO build quake 3 as a library that can be use for rendering embedded in other apps?

define DO_CLIENT_CC
  $(echo_cmd) "CLIENT_CC $<"
  $(Q)$(CC) $(CFLAGS) -o $@ -c $<
endef

define DO_AS
$(echo_cmd) "AS $<"
$(Q)$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
endef

define DO_TOOLS
$(echo_cmd) "TOOLS_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

define DO_WINDRES
$(echo_cmd) "WINDRES $<"
$(Q)$(WINDRES) -i $< -o $@
endef

debug:
	$(echo_cmd) "MAKE $(BD)/$(TARGET_CLIENT)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=client mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET_CLIENT)

release:
	$(echo_cmd) "MAKE $(BR)/$(TARGET_CLIENT)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=client mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET_CLIENT)

clean:
	@rm -rf $(BD)/client $(BD)/$(TARGET_CLIENT)
	@rm -rf $(BR)/client $(BR)/$(TARGET_CLIENT)

ifdef B
$(B)/client/%.o: code/client/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/unix/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/win32/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/macosx/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/wasm/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/sdl/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/qcommon/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/server/%.c
	$(DO_CLIENT_CC)

$(B)/$(TARGET_CLIENT): $(Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(CLIENT_LDFLAGS) $(LDFLAGS) 
endif
