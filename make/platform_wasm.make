HAVE_VM_COMPILED := false
BUILD_CLIENT     ?= 0
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
USE_RENDERER_DLOPEN := 0
USE_SYSTEM_JPEG  := 0
USE_SYSTEM_LIBC  := 0
USE_ABS_MOUSE    := 1
USE_LOCAL_DED    := 1
USE_LAZY_LOAD    := 1
USE_LAZY_MEMORY  := 1
USE_MASTER_LAN   := 1
USE_CURL         := 0
USE_SDL          := 0
USE_IPV6         := 0
USE_OPENGL2      := 1
USE_VULKAN       := 0

include make/configure.make

OPT              := wasm-opt
LD               := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/wasm-ld
CC               := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang
CXX              := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang++
BINEXT           := .wasm

SHLIBEXT         := wasm
SHLIBCFLAGS      := 
SHLIBLDFLAGS     := -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                    -Wl,--export-dynamic --no-standard-libraries \
                    -Wl,--no-entry -Wl,--allow-undefined-file=code/wasm/wasm.syms 
LDFLAGS          := -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                    -Wl,--export-dynamic --no-standard-libraries \
                    -Wl,--no-entry -Wl,--strip-all 

ifeq ($(BUILD_CLIENT),1)
SHLIBLDFLAGS     += -Wl,--allow-undefined-file=code/wasm/wasm.syms
LDFLAGS          += -Wl,--allow-undefined-file=code/wasm/wasm.syms
endif

ifeq ($(BUILD_RENDERER_OPENGL),1)
SHLIBLDFLAGS     += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
LDFLAGS          += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
endif

CLIENT_LDFLAGS   := 

BASE_CFLAGS      += -Wall --target=wasm32 --no-standard-libraries \
                    -Wimplicit -fstrict-aliasing \
                    -DGL_GLEXT_PROTOTYPES=1 \
                    -DGL_ARB_ES2_compatibility=1 \
                    -DGL_EXT_direct_state_access=1 \
                    -DUSE_Q3KEY \
                    -DUSE_MD5 \
                    -DUSE_ABS_MOUSE \
                    -DUSE_LOCAL_DED \
                    -DUSE_LAZY_LOAD \
                    -DUSE_LAZY_MEMORY \
                    -DUSE_MASTER_LAN \
                    -D__WASM__ \
                    -Icode/wasm/include 


DEBUG_CFLAGS     := $(BASE_CFLAGS) -fvisibility=default \
                    -std=c11 -DDEBUG -D_DEBUG -frtti -fPIC -O0 -g -g3 -gdwarf -gfull

RELEASE_CFLAGS   := $(BASE_CFLAGS) -fvisibility=hidden \
                    -std=c11 -DNDEBUG -O3 -Oz -flto -fPIC -Ofast

export INCLUDE   := -Icode/wasm/include 


ifdef B

pre-build:
	@:

post-build:
	$(Q)$(OPT) -Os --no-validation -o $@ $@
# TODO: compile all js files into one/minify/webpack
# TODO: insert bigchars font into web page

endif