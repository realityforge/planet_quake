HAVE_VM_COMPILED    := false
BUILD_CLIENT        ?= 0
BUILD_SERVER        := 0
BUILD_STANDALONE    := 1
USE_RENDERER_DLOPEN := 0
USE_SYSTEM_JPEG     := 0
USE_INTERNAL_JPEG   := 0
USE_INTERNAL_VORBIS := 1
USE_SYSTEM_LIBC     := 1
USE_CODEC_VORBIS    := 1
USE_CODEC_WAV       := 0
USE_ABS_MOUSE       := 1
USE_LOCAL_DED       := 0
USE_LAZY_LOAD       := 1
USE_LAZY_MEMORY     := 1
USE_MASTER_LAN      := 1
USE_CURL            := 0
USE_SDL             := 0
USE_IPV6            := 0
USE_OPENGL2         := 1
USE_VULKAN          := 0
USE_VULKAN_API      := 0
NO_MAKE_LOCAL       := 1

include make/configure.make

NODE                := node
COPY                := cp
UNLINK              := rm
MOVE                := mv
LD                  := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/wasm-ld
CC                  := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang
CXX                 := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang++
BINEXT              := .wasm

SHLIBEXT            := wasm
SHLIBCFLAGS         := -frtti -fPIC -MMD
SHLIBLDFLAGS        := -fPIC -Wl,-shared \
                       -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                       -Wl,--no-entry --no-standard-libraries -Wl,--export-dynamic

CLIENT_LDFLAGS      := -Wl,--import-memory -Wl,--import-table \
                       -Wl,--no-entry --no-standard-libraries \
												-Wl,--export-dynamic -Wl,--error-limit=200 \
												-Wl,--export=sprintf -Wl,--export=malloc  \
												-Wl,--export=stderr -Wl,--export=stdout  \
												-Wl,--export=errno --no-standard-libraries

RELEASE_LDFLAGS     := 
DEBUG_LDFLAGS       := -fvisibility=default -fno-inline

ifeq ($(BUILD_CLIENT),1)
SHLIBLDFLAGS        += -fvisibility=default -Wl,--allow-undefined-file=code/wasm/wasm.syms
CLIENT_LDFLAGS      += -Wl,--allow-undefined-file=code/wasm/wasm.syms
endif

ifeq ($(BUILD_RENDERER_OPENGL),1)
SHLIBLDFLAGS        += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
CLIENT_LDFLAGS      += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
endif

ifeq ($(BUILD_VORBIS),1)
SHLIBLDFLAGS        += -Wl,--allow-undefined-file=code/wasm/wasm-lib.syms
CLIENT_LDFLAGS      += -Wl,--allow-undefined-file=code/wasm/wasm-lib.syms
endif


ifndef BUILD_VORBIS
ifeq ($(USE_CODEC_VORBIS),1)
ifneq ($(USE_INTERNAL_VORBIS),1)
CLIENT_LDFLAGS      += -L$(INSTALL_FROM) -lvorbis_$(ARCH)
endif
endif
endif

CLIENT_LDFLAGS      += code/wasm/wasi/libclang_rt.builtins-wasm32.a \
	libs/wasi-sysroot/lib/wasm32-wasi/libc.a
# -fno-common -ffreestanding -nostdinc --no-standard-libraries
SDL_SOURCE          := libs/SDL2-2.0.14
BASE_CFLAGS         += -Wall --target=wasm32 \
                       -Wimplicit -fstrict-aliasing \
                       -ftree-vectorize -fsigned-char -MMD \
                       -ffast-math -fno-short-enums \
                       -Wno-extra-semi \
											-D_XOPEN_SOURCE=700 -D__EMSCRIPTEN__=1 \
											-D__WASM__=1 -D__wasi__=1 -D__wasm32__=1 \
											-D_WASI_EMULATED_SIGNAL -D_WASI_EMULATED_MMAN=1 \
                       -D_GNU_SOURCE=1 \
                       -DGL_GLEXT_PROTOTYPES=1 \
                       -DGL_ARB_ES2_compatibility=1 \
                       -DGL_EXT_direct_state_access=1 \
                       -DUSE_Q3KEY \
                       -DUSE_MD5 \
                       -DUSE_ABS_MOUSE \
                       -DUSE_LAZY_LOAD \
                       -DUSE_LAZY_MEMORY \
                       -DUSE_MASTER_LAN \
                       -D__WASM__ \
                       -std=gnu11 \
                       -I$(SDL_SOURCE)/include \
											 -Ilibs/wasi-sysroot/include \
                       -Icode/wasm
DEBUG_CFLAGS        := -fvisibility=default -fno-inline \
                       -DDEBUG -D_DEBUG -g -g3 -fPIC -gdwarf -gfull
RELEASE_CFLAGS      := -fvisibility=default  \
                       -DNDEBUG -Ofast -O3 -Oz -fPIC -ffast-math
# -flto
PK3_INCLUDES        := xxx-multigame-files.pk3  \
                       xxx-multigame-vms.pk3    \
											 lsdm3_v1-files.pk3
#                       lsdm3_v1-files.do-always       
#                       lsdm3_v1-images.do-always      


ifeq ($(filter $(MAKECMDGOALS),debug),debug)
WASM_TRGTDIR        := $(BD)
else
ifeq ($(filter $(MAKECMDGOALS),release),release)
WASM_TRGTDIR        := $(BR)
endif
endif
CLEANS              += $(subst .wasm,.html,$(notdir $(wildcard $(BD)/$(CNAME)*.wasm))) \
                       $(subst .wasm,.html,$(notdir $(wildcard $(BR)/$(CNAME)*.wasm)))
INDEX_OBJS          := $(WASM_TRGTDIR)/multigame/vm.do-always      \
                       $(BUILD_DIR)/xxx-multigame-sounds.do-always \
                       $(addprefix $(BUILD_DIR)/,$(subst .pk3,.do-always,$(PK3_INCLUDES)))


ifdef WASM_TRGTDIR
GAME_BUILD          := $(BUILD_DIR)/release-$(COMPILE_PLATFORM)-$(COMPILE_ARCH)

$(WASM_TRGTDIR)/multigame/vm.do-always:
	$(Q)$(MAKE) -f make/game_multi.make V=$(V) release \
		PLATFORM="$(COMPILE_PLATFORM)" BUILD_GAME_QVM=1  \
		B="$(GAME_BUILD)" ARCH="$(COMPILE_ARCH)"         \
		BUILD_GAME_LIB=0 

# TODO: YIKES, how is this more complicated than build.yml, why can't they be the same thing?
EXT01 := .do-always
$(BUILD_DIR)/%.do-always:
	$(eval REPACK_PATH := $(subst -sounds$(EXT01),$(EXT01),$(subst -vms$(EXT01),$(EXT01),$(subst -images$(EXT01),$(EXT01),$(subst -files$(EXT01),$(EXT01),$(notdir $@))))))
	$(eval REPACK_BUILD:= $(if $(filter %-files$(EXT01),$@),games/multigame/assets/$(subst $(EXT01),.pk3dir,$(REPACK_PATH)),$(if $(filter %-vms$(EXT01),$@),$(WASM_TRGTDIR)/multigame,$(BUILD_DIR)/$(subst $(EXT01),.pk3dir,$(REPACK_PATH)))))
	$(Q)$(if $(filter $(BUILD_DIR)/%.pk3dir,$(REPACK_BUILD)), \
	$(MAKE) -f make/build_package.make V=$(V) convert \
		SRCDIR="games/multigame/assets/$(subst $(EXT01),.pk3dir,$(REPACK_PATH))" \
		DESTDIR="$(BUILD_DIR)" TARGET_CONVERT="$(subst $(EXT01),.do-always,$(REPACK_PATH))" && \
	$(MAKE) -f make/build_package.make V=$(V) encode \
		SRCDIR="games/multigame/assets/$(subst $(EXT01),.pk3dir,$(REPACK_PATH))" \
		DESTDIR="$(BUILD_DIR)" TARGET_ENCODE="$(subst $(EXT01),.do-always,$(REPACK_PATH))")
	$(Q)$(MAKE) -f make/build_package.make V=$(V) package \
		SRCDIR="$(REPACK_BUILD)" \
		DESTDIR="$(BUILD_DIR)" \
		TARGET_REPACK="$(subst $(EXT01),.do-always,$(notdir $@))"

$(WASM_TRGTDIR)/%.html: $(WASM_TRGTDIR)/%.wasm

endif

index: $(INDEX_OBJS) ## create an index.html page out of the current build target
	$(MAKE) -f make/build_package.make V=$(V) index     \
		WASM_VFS="$(PK3_INCLUDES)"                        \
		STARTUP_COMMAND="+set\\', \\'developer\\', \\'1"  \
		DESTDIR="$(WASM_TRGTDIR)"
#		STARTUP_COMMAND="+set\\', \\'developer\\', \\'1\\', \\'+devmap\\', \\'lsdm3_v1" \


# TODO build quake 3 as a library that can be use for rendering embedded in other apps?
SDL_FLAGS := \
	-DSDL_VIDEO_DISABLED=1 -DSDL_JOYSTICK_DISABLED=1 \
	-DSDL_SENSOR_DISABLED=1 -DSDL_HAPTIC_DISABLED=1 \
	-DSDL_TIMER_UNIX=1 -DHAVE_MEMORY_H=1 -DHAVE_CLOCK_GETTIME=1 \
	-D_GNU_SOURCE=1 -DHAVE_STDLIB_H=1 -DHAVE_GETENV=0 \
	-DHAVE_UNISTD_H=1 -DHAVE_MATH_H=1 -DHAVE_M_PI=1 \
	-DHAVE_STDIO_H=1 -DHAVE_ALLOCA_H=1 -DHAVE_STRING_H=1 \
	-DSDL_THREADS_DISABLED=1 -DSDL_AUDIO_DRIVER_EMSCRIPTEN=1


define DO_SDL_CC
	$(echo_cmd) "SDL_CC $<"
	$(Q)$(CC) -o $@ -Wno-macro-redefined $(SDL_FLAGS) $(CLIENT_CFLAGS) -c $<
endef


# TODO: move this to make/lib_sdl.make
ifdef B
$(B)/client/%.o: $(SDL_SOURCE)/src/audio/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/audio/emscripten/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/events/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/atomic/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/thread/generic/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/thread/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/timer/unix/%.c
	$(DO_SDL_CC)

$(B)/client/%.o: $(SDL_SOURCE)/src/%.c
	$(DO_SDL_CC)
endif




# TODO: compile all js files into one/minify/webpack
# TODO: insert bigchars font into index page, insert all javascript and wasm into index page
# TODO: deploy index page with Actions
