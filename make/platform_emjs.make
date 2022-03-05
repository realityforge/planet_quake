USE_RENDERER_DLOPEN := 0
USE_BOTLIB_DLOPEN   := 0
USE_SYSTEM_JPEG     := 0
USE_INTERNAL_JPEG   := 1
USE_CURL            := 0
USE_OPENGL2         := 1
USE_VULKAN          := 0
NO_MAKE_LOCAL       := 1
BUILD_STANDALONE    := 1

include make/configure.make

ifndef EM_CACHE_LOCATION
EM_CACHE_LOCATION:= $(HOME)/.emscripten_cache
endif
ifndef EMJS_CONFIG_PATH
EMJS_CONFIG_PATH := $(HOME)/.emscripten
endif

CC               ?= emcc
BINEXT           := .js
SHLIBEXT         := wasm

HAVE_VM_COMPILED := true
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
BUILD_CLIENT     := 1

# curious if it combines then executes module linker
CLIENT_SYSTEM    := quake3e.js \
                    sys_fs.js \
                    sys_net.js \
                    sys_in.js \
                    sys_em.js

CLIENT_LIBS      := $(addprefix --js-library $(MOUNT_DIR)/wasm/http/,$(notdir $(CLIENT_SYSTEM)))


BASE_CFLAGS      += -Wall -Wno-unused-variable -fno-strict-aliasing \
                    -Wimplicit -Wstrict-prototypes \
                    -DGL_GLEXT_PROTOTYPES=1 -DGL_ARB_ES2_compatibility=1\
                    -DGL_EXT_direct_state_access=1 \
                    -DUSE_Q3KEY -DUSE_MD5 -D__WASM__ \
                    --em-config $(EMJS_CONFIG_PATH)

DEBUG_CFLAGS     := $(BASE_CFLAGS) \
                    -DDEBUG -D_DEBUG -frtti -fPIC -O0 -g -gsource-map
          
RELEASE_CFLAGS   := $(BASE_CFLAGS) \
                    -DNDEBUG -O3 -Oz -flto -fPIC

# TODO: IMPORTED_MEMORY
SHLIBLDFLAGS     += --no-entry \
                    -O0 -g -gsource-map -fPIC -gsource-map \
                    --source-map-base http://local.games:8080/ \
                    -DASSERTIONS \
                    -s WASM=1 \
                    -s STRICT=1 \
                    -s AUTO_JS_LIBRARIES=0 \
                    -s ALLOW_MEMORY_GROWTH=1 \
                    -s ERROR_ON_UNDEFINED_SYMBOLS=1 \
                    -s INCLUDE_FULL_LIBRARY=0 \
                    -s INVOKE_RUN=0 \
                    --em-config $(EMJS_CONFIG_PATH)


#SHLIBLDEXPORTS   = '__stack_pointer', '__memory_base', '__table_base', '__heap_base', '_free'
SHLIBLDEXPORTS    = '_free'
ifeq ($(BUILD_BOTLIB),1)
SHLIBLDFLAGS     += -s EXPORTED_FUNCTIONS="[$(SHLIBLDEXPORTS), '_GetBotLibAPI']"
endif

ifeq ($(BUILD_RENDERER_OPENGL2),1)
SHLIBLDFLAGS     += -s EXPORTED_FUNCTIONS="[$(SHLIBLDEXPORTS), '_GetRefAPI']"
endif

ifeq ($(USE_RENDERER_DLOPEN),0)
ifeq ($(USE_OPENGL2),1)
CLIENT_LDFLAGS   += -lGL \
                    -s MIN_WEBGL_VERSION=3 \
                    -s MAX_WEBGL_VERSION=3 \
                    -s FULL_ES2=1 \
                    -s FULL_ES3=1
else
CLIENT_LDFLAGS   += -lGL \
                    -s LEGACY_GL_EMULATION=1 \
                    -s MAX_WEBGL_VERSION=3 \
                    -s USE_WEBGL2=1
endif
endif

LDEXPORTS := '_free', '_BG_sprintf', '_malloc', '_RunGame', '_Z_Free', \
  '_Cvar_Set', '_Cvar_SetValue', '_Cvar_Get', '_Cbuf_AddText', \
  '_Cvar_VariableIntegerValue', '_FS_AllowedExtension', '_Com_Frame', \
  '_Cvar_VariableString', '_FS_GetCurrentGameDir', '_Sys_FileReady', \
  '_Key_KeynumToString', '_Sys_QueEvent', '_Sys_Milliseconds', \
  '_Key_ClearStates', '_gw_minimized', '_gw_active', '_g_bigcharsData', \
  '_g_bigcharsSize', '_glGetStringi', '_Cvar_SetIntegerValue', '_Key_GetCatcher', \
  '_Key_SetCatcher', '_FS_CopyString', '_Sys_Exit'

DEBUG_LDFLAGS    += -O0 -g -gsource-map \
                    -s WASM=1 \
                    -s MODULARIZE=0 \
                    -s SAFE_HEAP=1 \
                    -s STACK_OVERFLOW_CHECK=0 \
                    -s DEMANGLE_SUPPORT=1 \
                    -s ASSERTIONS=2 \
                    -s SINGLE_FILE=1 \
                    -DDEBUG -D_DEBUG \
                    -DASSERTIONS \
                    --source-map-base http://local.games:8080/ \
                    -s EXPORTED_FUNCTIONS=\"[$(LDEXPORTS), '_Z_MallocDebug', '_S_MallocDebug']\" \
                    -s DISABLE_EXCEPTION_CATCHING=0


RELEASE_LDFLAGS  += -O3 -Os \
                    -s WASM=1 \
                    -s MODULARIZE=0 \
                    -s SAFE_HEAP=0 \
                    -s DEMANGLE_SUPPORT=0 \
                    -s ASSERTIONS=0 \
                    -s SINGLE_FILE=1 \
                    -s EXPORTED_FUNCTIONS=\"[$(LDEXPORTS), '_Z_Malloc', '_S_Malloc']\" \
                    -s DISABLE_EXCEPTION_CATCHING=1

#-s EXPORTED_RUNTIME_METHODS="['FS', 'SYS', 'SYSC',  \
#  'SYSF', 'SYSN', 'SYSM', 'ccall', 'callMain', 'addFunction', \
#  'dynCall', 'UTF8ToString']" \
#-s STANDALONE_WASM=1 \

CLIENT_LDFLAGS   += $(CLIENT_LIBS) \
                    --post-js $(MOUNT_DIR)/wasm/sys_web.js \
                    --em-config $(EMJS_CONFIG_PATH) \
                    -s INITIAL_MEMORY=56MB \
                    -s ALLOW_MEMORY_GROWTH=1 \
                    -s ALLOW_TABLE_GROWTH=1 \
                    -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
                    -s FORCE_FILESYSTEM=0 \
                    -s EXPORTED_RUNTIME_METHODS="['SYS', 'GL']" \
                    -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE="[]" \
                    -s INCLUDE_FULL_LIBRARY=0 \
                    -s MAIN_MODULE=0 \
                    -s RELOCATABLE=0 \
                    -s EXPORT_ALL=0 \
                    -s LINKABLE=0 \
                    -s STRICT=1 \
                    -s AUTO_JS_LIBRARIES=0 \
                    --memory-init-file 0 \
                    -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1

#ifeq ($(USE_BOTLIB_DLOPEN),1)
#CLIENT_LDFLAGS += $(B)/$(BOTLIB_PREFIX)_libbots_$(SHLIBNAME)
#endif

#ifeq ($(USE_RENDERER_DLOPEN),1)
#CLIENT_LDFLAGS += $(B)/$(BOTLIB_PREFIX)_opengl2_$(SHLIBNAME)
#endif

export EM_CACHE=$(EM_CACHE_LOCATION)
#export EMCC_FORCE_STDLIBS=1

ifdef B
pre-build:
	@:
endif
