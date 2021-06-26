MKFILE           := $(lastword $(MAKEFILE_LIST))
WORKDIR          := client

BUILD_CLIENT     := 1
include make/platform.make

TARGET_CLIENT    := $(CNAME)$(ARCHEXT)$(BINEXT)
ifeq ($(USE_MULTIVM_CLIENT),1)
TARGET_CLIENT    := $(CNAME)_mw$(ARCHEXT)$(BINEXT)
endif
ifeq ($(BUILD_SLIM_CLIENT),1)
TARGET_CLIENT    := $(CNAME)_slim$(ARCHEXT)$(BINEXT)
endif

INCLUDES         := $(MOUNT_DIR)/qcommon
SOURCES          := $(MOUNT_DIR)/client

ifeq ($(BUILD_GAME_STATIC),1)
include make/game_baseq3a.make
endif

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
SOURCES          += $(MOUNT_DIR)/server
endif
ifneq ($(USE_BOTLIB_DLOPEN),1)
SOURCES          += $(MOUNT_DIR)/botlib
endif

CLIPMAP          := cm_load.o cm_patch.o cm_polylib.o cm_test.o cm_trace.o

QCOMMON          := cmd.o common.o cvar.o files.o history.o keys.o md4.o md5.o \
				            msg.o net_chan.o net_ip.o qrcodegen.o huffman.o \
				            huffman_static.o q_math.o q_shared.o unzip.o puff.o

# couple extra server files needed for cvars and botlib for reading files
ifeq ($(BUILD_SLIM_CLIENT),1)
QCOMMON          += sv_init.o sv_main.o sv_bot.o sv_game.o
endif

SOUND            := snd_adpcm.o snd_dma.o snd_mem.o snd_mix.o snd_wavelet.o \
						        snd_main.o snd_codec.o snd_codec_wav.o snd_codec_ogg.o \
						        snd_codec_opus.o

ifeq ($(ARCH),x86)
ifndef MINGW
SOUND            += snd_mix_mmx.o snd_mix_sse.o
endif
endif


VM               := vm.o
ifneq ($(BUILD_GAME_STATIC),1)
VM               += vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
VM               += vm_x86.o
endif
ifeq ($(ARCH),x86_64)
VM               += vm_x86.o
endif
ifeq ($(ARCH),arm)
VM               += vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
VM               += vm_aarch64.o
endif
endif
endif

CURL             :=
ifeq ($(USE_CURL),1)
#CURL     += cl_curl.o
ifneq ($(USE_CURL_DLOPEN),1)
CLIENT_LDFLAGS   += $(CURL_LIBS)
BASE_CFLAGS      += $(CURL_CFLAGS)
endif
endif


SYSTEM           := 

ifeq ($(PLATFORM),js)
SYSTEM           += sys_glimp.o sys_main.o sys_input.o sys_math.o
endif

ifneq ($(PLATFORM),js)
ifdef MINGW
SYSTEM           += win_main.o win_shared.o win_syscon.o win_resource.o
ifneq ($(USE_SDL),1)
SYSTEM           += win_gamma.o win_glimp.o win_input.o win_minimize.o \
                    win_qgl.o win_snd.o win_wndproc.o
endif
ifeq ($(USE_VULKAN_API),1)
SYSTEM           += win_qvk.o
endif
endif
ifndef MINGW
SYSTEM           += unix_main.o unix_shared.o linux_signals.o
ifneq ($(USE_SDL),1)
SYSTEM           += linux_glimp.o linux_qgl.o linux_snd.o \
                    x11_dga.o x11_randr.o x11_vidmode.o
endif
endif
endif

ifeq ($(USE_SDL),1)
SYSTEM           += sdl_glimp.o sdl_gamma.o sdl_input.o sdl_snd.o
endif

ifeq ($(USE_VULKAN_API),1)
SYSTEM           += linux_qvk.o
endif

VIDEO            :=
# TODO static linking? have to switch to gnu++
#ifeq ($(USE_CIN_VPX),1)
#VIDEO    += webmdec.o
#LIBS     += $(VPX_LIBS) $(VORBIS_LIBS) $(OPUS_LIBS)
#INCLUDES += libs/libvpx-1.10 \
					  libs/libvorbis-1.3.7/include \
					  libs/opus-1.3.1/include \
					  libs/libogg-1.3.4/include \
					  libs/libvpx-1.10/third_party/libwebm
#endif

ifeq ($(USE_RMLUI),1)
INCLUDES         += $(MOUNT_DIR)/../libs/RmlUi/Include
endif

CFILES           := $(foreach dir,$(SOURCES), $(wildcard $(dir)/cl_*.c)) \
                    $(CLIPMAP) $(QCOMMON) $(SOUND) $(VIDEO) $(VM) \
										$(CURL) $(SYSTEM)
            
ifneq ($(BUILD_SLIM_CLIENT),1)
ifneq ($(USE_BOTLIB_DLOPEN),1)
CFILES           += $(foreach dir,$(SOURCES), $(wildcard $(dir)/be_*.c)) \
						        $(foreach dir,$(SOURCES), $(wildcard $(dir)/l_*.c))
endif
#CFILES   += $(filter-out $(wildcard $(MOUNT_DIR)/server/sv_demo*.c),$(foreach dir,$(SOURCES), $(wildcard $(dir)/sv_*.c)))
CFILES           += $(foreach dir,$(SOURCES), $(wildcard $(dir)/sv_*.c))
else
CFILES           += $(MOUNT_DIR)/botlib/be_interface.c \
						        $(foreach dir,$(SOURCES), $(wildcard $(dir)/l_*.c))
endif
OBJS             := $(CFILES:.c=.o) 
LIBOBJ           ?= 
Q3OBJ            := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

ifeq ($(BUILD_GAME_STATIC),1)
Q3OBJ            += $(GAME_OBJ)
endif

ifneq ($(USE_RENDERER_DLOPEN),1)
ifneq ($(USE_OPENGL2),1)

else
ifneq ($(USE_VULKAN),1)
Q3OBJ            += $(REND_Q3OBJ)
else

endif
endif
endif

export INCLUDE   := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS           := $(INCLUDE) -fsigned-char -ftree-vectorize -ffast-math \
									  -fno-short-enums -MMD
ifeq ($(BUILD_GAME_STATIC),1)
CFLAGS           += $(GAME_INCLUDE)
endif

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
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS="$(WORKDIR) $(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET_CLIENT)

release:
	$(echo_cmd) "MAKE $(BR)/$(TARGET_CLIENT)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(WORKDIR) $(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET_CLIENT)

clean:
	@rm -rf ./$(BD)/$(WORKDIR) $(BD)/$(TARGET_CLIENT)
	@rm -rf ./$(BR)/$(WORKDIR) $(BR)/$(TARGET_CLIENT)
	@for i in $(CLEANS); \
	do \
	rm -r "./$(BD)/$$i" 2> /dev/null || true; \
	rm -r "./$(BR)/$$i" 2> /dev/null || true; \
	done

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

ifeq ($(PLATFORM),js)
$(B)/$(TARGET_CLIENT): $(Q3OBJ) $(LIBOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(LD) -o $@ $(Q3OBJ) $(LIBOBJ) $(CLIENT_LDFLAGS) $(LDFLAGS)
	
#$(B)/$(TARGET_CLIENT): $(Q3OBJ) $(LIBOBJ)
#	$(Q)llvm-link -o $(B)/$(CNAME)$(ARCHEXT).bc $(Q3OBJ) $(LIBOBJ)
#	$(Q)opt -Os $(B)/$(CNAME)$(ARCHEXT).bc -o $(B)/$(CNAME)$(ARCHEXT).bc
#	$(Q)llc -O3 -march=wasm32 -filetype=obj $(B)/$(CNAME)$(ARCHEXT).bc -o $(B)/$(CNAME)$(ARCHEXT).o
#	$(Q)$(LD) -o $@ $(B)/$(CNAME)$(ARCHEXT).o $(CLIENT_LDFLAGS) $(LDFLAGS)
#	$(Q)wasm-opt -Os --no-validation -o $@ $@

else
$(B)/$(TARGET_CLIENT): $(Q3OBJ) $(LIBOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(LIBOBJ) $(CLIENT_LDFLAGS) $(LDFLAGS) 
endif
endif
