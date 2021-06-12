MKFILE           := $(lastword $(MAKEFILE_LIST))
WORKDIR          := client

BUILD_CLIENT     := 1
include make/platform.make

TARGET_CLIENT    := $(CNAME)$(ARCHEXT)$(BINEXT)
ifeq ($(USE_MULTIVM_CLIENT),1)
TARGET_CLIENT    := $(CNAME)_mw$(ARCHEXT)$(BINEXT)
endif

INCLUDES := $(MOUNT_DIR)/qcommon
SOURCES  := $(MOUNT_DIR)/client

ifneq ($(USE_RENDERER_DLOPEN),1)
ifneq ($(USE_OPENGL2),1)
include make/build_renderer.make
else
ifneq ($(USE_VULKAN),1)
include make/build_renderer2.make
else
include make/build_renderervk.make
endif
endif
endif

ifneq ($(BUILD_SLIM_CLIENT),1)
SOURCES  += $(MOUNT_DIR)/server
endif
ifneq ($(USE_BOTLIB_DLOPEN),1)
SOURCES  += $(MOUNT_DIR)/botlib
endif

CLIPMAP  := $(B)/client/cm_load.o \
            $(B)/client/cm_load_bsp2.o \
            $(B)/client/cm_patch.o \
            $(B)/client/cm_polylib.o \
            $(B)/client/cm_test.o \
            $(B)/client/cm_trace.o

QCOMMON  := $(B)/client/cmd.o \
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
            $(B)/client/puff.o

# couple extra server files needed for cvars and botlib for reading files
ifeq ($(BUILD_SLIM_CLIENT),1)
QCOMMON  += $(B)/client/sv_init.o \
            $(B)/client/sv_main.o \
            $(B)/client/sv_bot.o \
            $(B)/client/sv_game.o
endif

SOUND    := $(B)/client/snd_adpcm.o \
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
SOUND    += $(B)/client/snd_mix_mmx.o \
            $(B)/client/snd_mix_sse.o
endif
endif


VM       := $(B)/client/vm.o \
            $(B)/client/vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
VM       += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),x86_64)
VM       += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),arm)
VM       += $(B)/client/vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
VM       += $(B)/client/vm_aarch64.o
endif
endif

CURL     :=
ifeq ($(USE_CURL),1)
#CURL     += $(B)/client/cl_curl.o
ifneq ($(USE_CURL_DLOPEN),1)
CLIENT_LDFLAGS += $(CURL_LIBS)
BASE_CFLAGS    += $(CURL_CFLAGS)
endif
endif


SYSTEM   :=
ifeq ($(PLATFORM),js)
SYSTEM   += $(B)/client/sys_glimp.o \
            $(B)/client/sys_main.o \
            $(B)/client/sys_input.o \
            $(B)/client/unix_shared.o

else
ifdef MINGW
SYSTEM   += $(B)/client/win_main.o \
            $(B)/client/win_shared.o \
            $(B)/client/win_syscon.o \
            $(B)/client/win_resource.o

ifeq ($(USE_SDL),1)
SYSTEM   += $(B)/client/sdl_glimp.o \
            $(B)/client/sdl_gamma.o \
            $(B)/client/sdl_input.o \
            $(B)/client/sdl_snd.o

else # !USE_SDL
SYSTEM   += $(B)/client/win_gamma.o \
            $(B)/client/win_glimp.o \
            $(B)/client/win_input.o \
            $(B)/client/win_minimize.o \
            $(B)/client/win_qgl.o \
            $(B)/client/win_snd.o \
            $(B)/client/win_wndproc.o
ifeq ($(USE_VULKAN_API),1)
SYSTEM   += $(B)/client/win_qvk.o
endif
endif # !USE_SDL
else # !MINGW
SYSTEM   += $(B)/client/unix_main.o \
            $(B)/client/unix_shared.o \
            $(B)/client/linux_signals.o

ifeq ($(USE_SDL),1)
SYSTEM   += $(B)/client/sdl_glimp.o \
            $(B)/client/sdl_gamma.o \
            $(B)/client/sdl_input.o \
            $(B)/client/sdl_snd.o
else # !USE_SDL
SYSTEM   += $(B)/client/linux_glimp.o \
            $(B)/client/linux_qgl.o \
            $(B)/client/linux_snd.o \
            $(B)/client/x11_dga.o \
            $(B)/client/x11_randr.o \
            $(B)/client/x11_vidmode.o
endif

ifeq ($(USE_VULKAN_API),1)
SYSTEM   += $(B)/client/linux_qvk.o
endif
endif # !USE_SDL
endif

VIDEO    :=
# TODO static linking? have to switch to gnu++
#ifeq ($(USE_CIN_VPX),1)
#VIDEO    += $(B)/client/webmdec.o
#LIBS     += $(VPX_LIBS) $(VORBIS_LIBS) $(OPUS_LIBS)
#INCLUDES += libs/libvpx-1.10 \
					  libs/libvorbis-1.3.7/include \
					  libs/opus-1.3.1/include \
					  libs/libogg-1.3.4/include \
					  libs/libvpx-1.10/third_party/libwebm
#endif

ifeq ($(USE_RMLUI),1)
INCLUDES += $(MOUNT_DIR)/../libs/RmlUi/Include
endif

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/cl_*.c)) \
            $(CLIPMAP) $(QCOMMON) $(SOUND) $(VIDEO) \
            $(VM) $(CURL) $(SYSTEM)
ifneq ($(BUILD_SLIM_CLIENT),1)
ifneq ($(USE_BOTLIB_DLOPEN),1)
CFILES   += $(foreach dir,$(SOURCES), $(wildcard $(dir)/be_*.c)) \
						$(foreach dir,$(SOURCES), $(wildcard $(dir)/l_*.c))
endif
#CFILES   += $(filter-out $(wildcard $(MOUNT_DIR)/server/sv_demo*.c),$(foreach dir,$(SOURCES), $(wildcard $(dir)/sv_*.c)))
CFILES   += $(foreach dir,$(SOURCES), $(wildcard $(dir)/sv_*.c))
else
CFILES   += $(MOUNT_DIR)/botlib/be_interface.c \
						$(foreach dir,$(SOURCES), $(wildcard $(dir)/l_*.c))
endif
OBJS     := $(CFILES:.c=.o) 
Q3OBJ    := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

ifneq ($(USE_RENDERER_DLOPEN),1)
ifneq ($(USE_OPENGL2),1)

else
ifneq ($(USE_VULKAN),1)
Q3OBJ    += $(REND_Q3OBJ)
else

endif
endif
endif

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS   := $(INCLUDE) -fsigned-char -ftree-vectorize \
            -ffast-math -fno-short-enums -MMD
#GXXFLAGS := $(CFLAGS) -std=gnu++11

# TODO build quake 3 as a library that can be use for rendering embedded in other apps?

define DO_CLIENT_CC
  $(echo_cmd) "CLIENT_CC $<"
  $(Q)$(CC) -o $@ $(CFLAGS) -c $<
endef

define DO_BOT_CC
	$(echo_cmd) "BOT_CC $<"
	$(Q)$(CC) -o $@ $(CFLAGS) -DBOTLIB -c $<
endef

define DO_SERVER_CC
  $(echo_cmd) "SERVER_CC $<"
  $(Q)$(CC) $(CFLAGS) -o $@ -c $<
endef

#define DO_VPX_GXX
#  $(echo_cmd) "VPX_GXX $<"
#  $(Q)$(GXX) -o $@ $(GXXFLAGS) -c $<
#endef

# TODO: use these
#define DO_AS
#$(echo_cmd) "AS $<"
#$(Q)$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
#endef

#define DO_TOOLS
#$(echo_cmd) "TOOLS_CC $<"
#$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
#endef

#define DO_WINDRES
#$(echo_cmd) "WINDRES $<"
#$(Q)$(WINDRES) -i $< -o $@
#endef

debug:
	$(echo_cmd) "MAKE $(BD)/$(TARGET_CLIENT)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET_CLIENT)

release:
	$(echo_cmd) "MAKE $(BR)/$(TARGET_CLIENT)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET_CLIENT)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET_CLIENT)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET_CLIENT)

ifdef B
$(B)/$(WORKDIR)/%.o: libs/libvpx-1.10/%.cc
	$(DO_VPX_GXX)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/client/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/unix/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/win32/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/macosx/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/wasm/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/sdl/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_CLIENT_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/server/%.c
	$(DO_SERVER_CC)

$(B)/$(WORKDIR)/%.o: $(MOUNT_DIR)/botlib/%.c
	$(DO_BOT_CC)

$(B)/$(TARGET_CLIENT): $(Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(CLIENT_LDFLAGS) $(LDFLAGS) 
endif
