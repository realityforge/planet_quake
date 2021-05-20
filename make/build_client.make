MKFILE      := $(lastword $(MAKEFILE_LIST)) 

include make/platform.make
include make/configure.make
include make/platform_os.make

RENDERER_PREFIX  := $(CNAME)
TARGET	         := $(CNAME)

SOURCES  := $(MOUNT_DIR)/client

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
					  $(B)/client/puff.o \
						$(B)/client/sv_init.o \
						$(B)/client/sv_main.o \
						$(B)/client/sv_bot.o \
						$(B)/client/sv_game.o

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
  SOUND   += \
					  $(B)/client/snd_mix_mmx.o \
					  $(B)/client/snd_mix_sse.o
endif
endif
	  
VM   := \
					$(B)/client/vm.o \
					$(B)/client/vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
  VM += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),x86_64)
  VM += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),arm)
  VM += $(B)/client/vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
  VM += $(B)/client/vm_aarch64.o
endif
endif

CURL   :=
ifeq ($(USE_CURL),1)
  CURL += $(B)/client/cl_curl.o
endif


Q3OBJ   :=
ifdef MINGW
  Q3OBJ += \
    $(B)/client/win_main.o \
    $(B)/client/win_shared.o \
    $(B)/client/win_syscon.o \
    $(B)/client/win_resource.o

ifeq ($(USE_SDL),1)
ifneq ($(PLATFORM),js)
  Q3OBJ += \
    $(B)/client/sdl_glimp.o \
    $(B)/client/sdl_gamma.o \
    $(B)/client/sdl_input.o \
    $(B)/client/sdl_snd.o
endif

else # !USE_SDL
  Q3OBJ += \
    $(B)/client/win_gamma.o \
    $(B)/client/win_glimp.o \
    $(B)/client/win_input.o \
    $(B)/client/win_minimize.o \
    $(B)/client/win_qgl.o \
    $(B)/client/win_snd.o \
    $(B)/client/win_wndproc.o
ifeq ($(USE_VULKAN_API),1)
  Q3OBJ += \
      $(B)/client/win_qvk.o
endif
endif # !USE_SDL

else # !MINGW
ifeq ($(PLATFORM),js)
  Q3OBJ += \
		$(B)/client/sdl_glimp.o \
		$(B)/client/sdl_gamma.o \
		$(B)/client/sdl_snd.o \
		$(B)/client/sys_main.o \
		$(B)/client/sys_input.o

else
  Q3OBJ += \
    $(B)/client/unix_main.o \
    $(B)/client/unix_shared.o \
    $(B)/client/linux_signals.o
endif

ifeq ($(USE_SDL),1)
ifneq ($(PLATFORM),js)
  Q3OBJ += \
    $(B)/client/sdl_glimp.o \
    $(B)/client/sdl_gamma.o \
    $(B)/client/sdl_input.o \
    $(B)/client/sdl_snd.o
endif
else # !USE_SDL
  Q3OBJ += \
    $(B)/client/linux_glimp.o \
    $(B)/client/linux_qgl.o \
    $(B)/client/linux_snd.o \
    $(B)/client/x11_dga.o \
    $(B)/client/x11_randr.o \
    $(B)/client/x11_vidmode.o

ifeq ($(USE_VULKAN_API),1)
  Q3OBJ += \
      $(B)/client/linux_qvk.o
endif
endif # !USE_SDL
endif


INCLUDES       := 
#LIBS          := -l
CFILES         := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
									$(CLIPMAP) \
									$(QCOMMON) \
									$(SOUND) \
									$(VM) \
									$(CURL) \
									$(Q3OBJ)
OBJS           := $(CFILES:.c=.o) 
Q3OBJ          := $(addprefix $(B)/client/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS   := $(INCLUDE) -fsigned-char \
             -O2 -ftree-vectorize -g -ffast-math -fno-short-enums \
						 -MMD -DBUILD_SLIM_CLIENT \
						 -DUSE_RENDERER_DLOPEN \
						 -DRENDERER_PREFIX=\"$(RENDERER_PREFIX)\"
#						 -DUSE_SYSTEM_JPEG
#LDFLAGS  := -L$(MOUNT_DIR)/macosx -lxml2 -lpng \
						$(MOUNT_DIR)/macosx/libxml2.2.dylib $(MOUNT_DIR)/macosx/libpng.dylib \
						-L$(MOUNT_DIR)/macosx -I$(MOUNT_DIR)/RmlUi/Include
LDFLAGS  := -L$(BD) -ljpeg \
						$(BD)/quake3e_libbots_x86_64.dylib

# TODO build quake 3 as a library that can be use for rendering embedded in other apps?

define DO_CLIENT_CC
	$(echo_cmd) "CLIENT_CC $<"
	$(Q)$(CC) $(CFLAGS) $(SDL_INCLUDE) -o $@ -c $<
endef

define DO_AS
$(echo_cmd) "AS $<"
$(Q)$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
endef

define DO_TOOLS
$(echo_cmd) "TOOLS_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -I$(TDIR)/libs -I$(TDIR)/include -I$(TDIR)/common -o $@ -c $<
endef

define DO_WINDRES
$(echo_cmd) "WINDRES $<"
$(Q)$(WINDRES) -i $< -o $@
endef

mkdirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/client ];then $(MKDIR) $(B)/client;fi
# TODO: make all these dylibs
#	@if [ ! -d $(B)/client/asm ];then $(MKDIR) $(B)/client/asm;fi
#	@if [ ! -d $(B)/client/ogg ];then $(MKDIR) $(B)/client/ogg;fi
#	@if [ ! -d $(B)/client/vorbis ];then $(MKDIR) $(B)/client/vorbis;fi
#	@if [ ! -d $(B)/client/opus ];then $(MKDIR) $(B)/client/opus;fi
#	@if [ ! -d $(B)/client/q3map2 ];then $(MKDIR) $(B)/client/q3map2;fi
#	@if [ ! -d $(B)/client/tools ];then $(MKDIR) $(B)/client/tools;fi
#	@if [ ! -d $(B)/client/libs ];then $(MKDIR) $(B)/client/libs;fi

default:
	$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)

#debug:
#	@$(MAKE) -f $(MKFILE) $(TARGETS) B=$(BD) CFLAGS="$(CFLAGS) $(BASE_CFLAGS)" \
#	  OPTIMIZE="$(DEBUG_CFLAGS)" V=$(V)

#release:
#	@$(MAKE) -f $(MKFILE) $(TARGETS) B=$(BR) CFLAGS="$(CFLAGS) $(BASE_CFLAGS)" \
#	  OPTIMIZE="-DNDEBUG $(OPTIMIZE)" V=$(V)

clean:
	@rm -rf $(BD)/client
	@rm -rf $(BR)/client

ifdef B

$(B)/client/%.o: code/client/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/unix/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/win32/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/macosx/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/sdl/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/qcommon/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/server/%.c
	$(DO_CLIENT_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $^ $(LIBS) $(LDFLAGS) $(SDL_LIBS) -o $@

D_FILES=$(shell find $(BD)/client -name '*.d')
endif

ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean makedirs release \
  targets tools toolsclean mkdirs \
	$(D_FILES)

.DEFAULT_GOAL := default
