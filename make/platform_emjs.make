BUILD_SERVER     := 0
BUILD_STANDALONE := 1
HOME          := 
EMSDK         := /Users/briancullinan/planet_quake/libs/emsdk
NODE_JS       := $(EMSDK)/node/12.9.1_64bit/bin/node
BINARYEN_ROOT := $(EMSDK)/upstream
EMSCRIPTEN    := $(EMSDK)/upstream/emscripten
LLVM_ROOT     := $(EMSDK)/upstream/bin
EM_CONFIG     := LLVM_ROOT=\'$(EMSDK)/upstream/bin\'\;\\nNODE_JS=\'$(NODE_JS)\'\;\\nBINARYEN_ROOT=\'$(BINARYEN_ROOT)\'\;\\nEMSCRIPTEN_ROOT=\'$(EMSCRIPTEN)\'\;
ifndef EMSCRIPTEN_CACHE
EMSCRIPTEN_CACHE := $(HOME)/.emscripten_cache
endif

pre-build:
	@echo $(EM_CONFIG) > $(B)/.emscripten

CC=$(EMSCRIPTEN)/emcc
RANLIB=$(EMSCRIPTEN)/emranlib
ARCH=js
BINEXT=.wasm
STRIP=echo

DEBUG=0
EMCC_DEBUG=0

HAVE_VM_COMPILED=true
BUILD_CLIENT=1
BUILD_GAME_QVM=1
BUILD_GAME_SO=0
BUILD_RENDERER_OPENGL=0
BUILD_RENDERER_JS=0
BUILD_RENDERER_OPENGL2=1
BUILD_RENDERER_OPENGLES=0

USE_Q3KEY=1
USE_IPV6=0
USE_SDL=1
USE_VULKAN=0
USE_CURL=0
USE_CURL_DLOPEN=0
USE_CODEC_VORBIS=1
USE_CODEC_OPUS=0
USE_FREETYPE=0
USE_MUMBLE=0
USE_VOIP=0
SDL_LOADSO_DLOPEN=0
USE_OPENAL_DLOPEN=0
USE_RENDERER_DLOPEN=0
USE_LOCAL_HEADERS=0
GL_EXT_direct_state_access=1
GL_ARB_ES2_compatibility=1
GL_GLEXT_PROTOTYPES=1

BASE_CFLAGS = \
          -Wall -Wno-unused-variable -fno-strict-aliasing \
          -Wimplicit -Wstrict-prototypes \
          -DGL_GLEXT_PROTOTYPES=1 -DGL_ARB_ES2_compatibility=1\
          -DGL_EXT_direct_state_access=1 \
          -DUSE_Q3KEY -DUSE_MD5

SHLIBCFLAGS = \
          -DEMSCRIPTEN \
          -fvisibility=hidden \
          -O1 -g3 \
          -s STRICT=1 \
          -s AUTO_JS_LIBRARIES=0 \
          -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
          -s SIDE_MODULE=1 \
          -s RELOCATABLE=1 \
          -s LINKABLE=1 \
          -s EXPORT_ALL=1 \
          -s EXPORTED_FUNCTIONS="['_GetRefAPI']" \
          -s ALLOW_TABLE_GROWTH=1 \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s GL_UNSAFE_OPTS=0 \
          -s LEGACY_GL_EMULATION=0 \
          -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1 \
          -s MIN_WEBGL_VERSION=1 \
          -s MAX_WEBGL_VERSION=3 \
          -s USE_WEBGL2=1 \
          -s FULL_ES2=1 \
          -s FULL_ES3=1 \
          -s USE_SDL=2 \
          -s EXPORT_NAME=\"quake3e_opengl2_js\" \
          -s WASM=0 \
          -s MODULARIZE=0 \
          -s SAFE_HEAP=1 \
          -s DEMANGLE_SUPPORT=1 \
          -s ASSERTIONS=1 \
          -frtti \
          -fPIC

ifeq ($(USE_RENDERER_DLOPEN),1)
CLIENT_LDFLAGS += \
          -s EXPORT_ALL=1 \
          -s RELOCATABLE=1 \
          -s DECLARE_ASM_MODULE_EXPORTS=1 \
          -s LINKABLE=1 \
          -s INCLUDE_FULL_LIBRARY=1
endif

SHLIBEXT=js

#  --llvm-lto 3
#   -s USE_WEBGL2=1
#   -s MIN_WEBGL_VERSION=2
#   -s MAX_WEBGL_VERSION=2
#   -s USE_SDL_IMAGE=2 \
#   -s SDL2_IMAGE_FORMATS='["bmp","png","xpm"]' \
# --em-config $(EM_CONFIG) \
# --cache $(EMSCRIPTEN_CACHE) \
#    -s INITIAL_MEMORY=56MB \

CLIENT_LDFLAGS += \
          -lbrowser.js \
          -lasync.js \
          -lidbfs.js \
          -lsdl.js \
          --js-library $(QUAKEJS)/sys_common.js \
          --js-library $(QUAKEJS)/sys_browser.js \
          --js-library $(QUAKEJS)/sys_net.js \
          --js-library $(QUAKEJS)/sys_files.js \
          --js-library $(QUAKEJS)/sys_input.js \
          --js-library $(QUAKEJS)/sys_main.js \
          --js-library $(CMDIR)/vm_js.js \
          --pre-js $(QUAKEJS)/sys_polyfill.js \
          --post-js $(QUAKEJS)/sys_overrides.js \
          -s MINIMAL_RUNTIME=0 \
          -s STRICT=1 \
          -s MAIN_MODULE=0 \
          -s AUTO_JS_LIBRARIES=1 \
          -s ALLOW_TABLE_GROWTH=1 \
          -s INITIAL_MEMORY=200MB \
          -s ALLOW_MEMORY_GROWTH=1 \
          --memory-init-file 0 \
          \
          -s DISABLE_EXCEPTION_CATCHING=0 \
          -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 \
          -s ERROR_ON_UNDEFINED_SYMBOLS=1 \
          -s INVOKE_RUN=1 \
          -s NO_EXIT_RUNTIME=1 \
          -s EXIT_RUNTIME=1 \
          -s EXTRA_EXPORTED_RUNTIME_METHODS="['FS', 'SYS', 'SYSC', 'SYSF', 'SYSN', 'SYSM', 'ccall', 'callMain', 'addFunction', 'dynCall']" \
          -s EXPORTED_FUNCTIONS="['_main', '_malloc', '_free', '_atof', '_strncpy', '_memset', '_memcpy', '_fopen', '_fseek', '_Com_WriteConfigToFile', '_IN_PushInit', '_IN_PushEvent', '_S_DisableSounds', '_CL_GetClientState', '_Com_Printf', '_CL_Outside_NextDownload', '_NET_SendLoopPacket', '_SOCKS_Frame_Proxy', '_Com_Frame_Proxy', '_Com_Outside_Error', '_Z_Malloc', '_Z_Free', '_S_Malloc', '_Cvar_Set', '_Cvar_SetValue', '_Cvar_Get', '_Cvar_VariableString', '_Cvar_VariableIntegerValue', '_Cbuf_ExecuteText', '_Cbuf_Execute', '_Cbuf_AddText', '_Field_CharEvent']" \
          -s GL_UNSAFE_OPTS=0 \
          -s USE_SDL=2 \
          -s USE_SDL_MIXER=2 \
          -s USE_VORBIS=1 \
          -s USE_OGG=1 \
          -s USE_PTHREADS=0 \
          -s FORCE_FILESYSTEM=1 \
          -s EXPORT_NAME=\"quake3e\"
          
ifeq ($(BUILD_RENDERER_OPENGL2),1)
CLIENT_LDFLAGS += \
          -lwebgl.js \
          -lwebgl2.js \
          -s LEGACY_GL_EMULATION=0 \
          -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1 \
          -s MIN_WEBGL_VERSION=1 \
          -s MAX_WEBGL_VERSION=3 \
          -s USE_WEBGL2=1 \
          -s FULL_ES2=1 \
          -s FULL_ES3=1
endif

ifeq ($(BUILD_RENDERER_OPENGL),1)
CLIENT_LDFLAGS += \
          -lglemu.js \
          -lwebgl.js \
          -DUSE_CLOSURE_COMPILER \
          -s LEGACY_GL_EMULATION=1 \
          -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1 \
          -s MIN_WEBGL_VERSION=1 \
          -s MAX_WEBGL_VERSION=3 \
          -s USE_WEBGL2=1 \
          -s FULL_ES2=0 \
          -s FULL_ES3=0
endif

ifneq ($(USE_CODEC_VORBIS),0)
CLIENT_LDFLAGS += -lvorbis -logg
BASE_CFLAGS    += -DUSE_CODEC_VORBIS=1 \
            	-I$(OGGDIR)/ \
            	-I$(VORBISDIR)/
endif

# debug optimize flags: --closure 0 --minify 0 -g -g4 || -O1 --closure 0 --minify 0 -g -g3
# -DDEBUG -D_DEBUG
DEBUG_CFLAGS := $(BASE_CFLAGS) \
          -O1 -g3 \
          -frtti \
          -flto \
          -fPIC \
          --em-config $(BD)/.emscripten
RELEASE_CFLAGS += --em-config $(BR)/.emscripten

ifeq ($(DEBUG), 1)
CLIENT_LDFLAGS += \
          -s WASM=1 \
          -s MODULARIZE=0 \
          -s SAFE_HEAP=0 \
          -s DEMANGLE_SUPPORT=1 \
          -s ASSERTIONS=2 \
          -s SINGLE_FILE=1
else
CLIENT_LDFLAGS += \
          -s WASM=1 \
          -s MODULARIZE=0 \
          -s SAFE_HEAP=0 \
          -s DEMANGLE_SUPPORT=0 \
          -s ASSERTIONS=2
endif

RELEASE_CFLAGS=$(BASE_CFLAGS) \
          -DNDEBUG \
          -O3 -Oz \
          -flto \
          -fPIC

ifneq ($(USE_CODEC_OPUS),0)
CLIENT_LDFLAGS += -lopus
RELEASE_CFLAGS += \
          -DUSE_CODEC_OPUS=1 \
          -DOPUS_BUILD -DHAVE_LRINTF -DFLOATING_POINT -DFLOAT_APPROX -DUSE_ALLOCA \
          -I$(OPUSDIR)/include \
          -I$(OPUSDIR)/celt \
          -I$(OPUSDIR)/silk \
          -I$(OPUSDIR)/silk/float \
          -I$(OPUSFILEDIR)/include 
endif
