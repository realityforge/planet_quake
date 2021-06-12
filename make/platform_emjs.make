USE_RENDERER_DLOPEN := 1
USE_BOTLIB_DLOPEN   := 1

include make/configure.make

EMSDK            := $(ABSOLUTE_PATH)/libs/emsdk
ifndef EM_CACHE_LOCATION
EM_CACHE_LOCATION:= $(HOME)/.emscripten_cache
endif
EMJS_CONFIG_PATH := $(ABSOLUTE_PATH)/build/.emscripten

CC               := $(EMSDK)/upstream/emscripten/emcc
RANLIB           := $(EMSDK)/upstream/emscripten/emranlib
BINEXT           := .js
SHLIBEXT         := wasm

HAVE_VM_COMPILED := true
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
BUILD_CLIENT     := 1

CLIENT_SYSTEM    := sys_common.js \
                    sys_browser.js \
                    sys_net.js \
                    sys_files.js \
                    sys_input.js \
                    sys_webgl.js \
									  sys_sdl.js \
                    sys_main.js

CLIENT_LIBS      := -lbrowser.js \
                    -lasync.js \
                    -lidbfs.js \
                    -ldylink.js \
                    $(addprefix --js-library $(MOUNT_DIR)/wasm/,$(notdir $(CLIENT_SYSTEM))) \
                    --js-library $(MOUNT_DIR)/qcommon/vm_js.js \
                    --pre-js $(MOUNT_DIR)/wasm/sys_polyfill.js \
                    --post-js $(MOUNT_DIR)/wasm/sys_overrides.js

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
                    -s MODULARIZE=0 \
                    -s STANDALONE_WASM=0 \
                    -s IMPORTED_MEMORY=0 \
                    -s SIDE_MODULE=1 \
                    -s RELOCATABLE=1 \
                    -s LINKABLE=0 \
                    -s USE_PTHREADS=0 \
									  -s USE_SDL=0 \
                    -s INVOKE_RUN=0 \
                    --shared-memory \
                    --enable-threads \
                    --em-config $(EMJS_CONFIG_PATH)


#SHLIBLDEXPORTS   = '__stack_pointer', '__memory_base', '__table_base', '__heap_base', '_free'
SHLIBLDEXPORTS    = '_free'
ifeq ($(BUILD_BOTLIB),1)
SHLIBLDFLAGS     += -s EXPORTED_FUNCTIONS="[$(SHLIBLDEXPORTS), '_GetBotLibAPI']"
endif

ifeq ($(BUILD_RENDERER_OPENGL2),1)
SHLIBLDFLAGS     += -s EXPORTED_FUNCTIONS="[$(SHLIBLDEXPORTS), '_GetRefAPI']"
endif

ifeq ($(BUILD_RENDERER_OPENGL),1)
# CLIENT_LDFLAGS += \
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

SYSEXPORTS := , '_Sys_BeginDownload', '_Sys_SocksConnect', '_Sys_SocksMessage', \
							'_Sys_NET_MulticastLocal', '_Sys_LoadLibrary', '_Sys_DownloadLocalFile', \
							'_Sys_CmdArgsC', '_Sys_CmdArgs'

LDEXPORTS := '_main', '_malloc', '_free', '_atof', '_sin', '_powf', '_rand', \
  '_strncpy', '_memset', '_memcpy', '_fopen', '_fseek', '_dlopen', '_vsprintf', \
  '_IN_PushInit', '_IN_PushEvent', '_sscanf', '_Cvar_VariableString', \
  '_S_DisableSounds', '_CL_GetClientState', '_Com_Printf', '_strstr', '_strcpy', \
  '_CL_Outside_NextDownload', '_NET_SendLoopPacket', '_SOCKS_Frame_Proxy', \
  '_Com_Frame_Proxy', '_Com_Outside_Error', '_Z_Free', '_Cbuf_Execute', \
  '_Cvar_Set', '_Cvar_SetValue', '_Cvar_Get', '_Cbuf_ExecuteText', \
  '_Cvar_VariableIntegerValue', '_Cbuf_AddText', '_JS_Field_CharEvent', \
	'_FS_AllowedExtension' $(SYSEXPORTS)

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
                    -s EXPORTED_FUNCTIONS=\"[$(LDEXPORTS), '_Z_MallocDebug', '_S_MallocDebug']\"

RELEASE_LDFLAGS  += -s WASM=1 \
                    -s MODULARIZE=0 \
                    -s SAFE_HEAP=1 \
                    -s DEMANGLE_SUPPORT=0 \
                    -s ASSERTIONS=0 \
                    -s SINGLE_FILE=1 \
                    -s EXPORTED_FUNCTIONS=\"[$(LDEXPORTS), '_Z_Malloc', '_S_Malloc']\"

CLIENT_LDFLAGS   += $(CLIENT_LIBS) \
                    --em-config $(EMJS_CONFIG_PATH) \
                    -s INITIAL_MEMORY=56MB \
                    -s ALLOW_MEMORY_GROWTH=1 \
                    -s ALLOW_TABLE_GROWTH=1 \
                    -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
                    -s EXPORTED_RUNTIME_METHODS="['FS', 'SYS', 'SYSC',  \
                      'SYSF', 'SYSN', 'SYSM', 'ccall', 'callMain', 'addFunction', \
                      'dynCall', 'UTF8ToString']" \
                    -s FORCE_FILESYSTEM=1 \
                    -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE="[]" \
                    -s INCLUDE_FULL_LIBRARY=0 \
                    -s MAIN_MODULE=0 \
                    -s RELOCATABLE=0 \
                    -s EXPORT_ALL=0 \
                    -s LINKABLE=0 \
                    -s STRICT=1 \
                    -s AUTO_JS_LIBRARIES=0 \
                    --memory-init-file 0 \
                    -s DISABLE_EXCEPTION_CATCHING=0 \
                    -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 \
                    -s INVOKE_RUN=1 \
                    -s EXIT_RUNTIME=1

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
	@echo "" > $(EMJS_CONFIG_PATH)
	@echo "LLVM_ROOT         = '$(EMSDK)/upstream/bin';\\n"                >> $(EMJS_CONFIG_PATH)
	@echo "NODE_JS           = '$(EMSDK)/node/14.15.5_64bit/bin/node';\\n" >> $(EMJS_CONFIG_PATH)
	@echo "BINARYEN_ROOT     = '$(EMSDK)/upstream';\\n"                    >> $(EMJS_CONFIG_PATH)
	@echo "__WASM___ROOT   = '$(EMSDK)/upstream/emscripten';"            >> $(EMJS_CONFIG_PATH)
endif
