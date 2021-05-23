ABSOLUTE_PATH    := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))..
EMSDK            := $(ABSOLUTE_PATH)/libs/emsdk
ifndef EM_CACHE
EM_CACHE         := $(HOME)/.emscripten_cache
@shell export EM_CACHE=$(EM_CACHE)
endif
EMJS_CONFIG_PATH := $(ABSOLUTE_PATH)/build/.emscripten

CC               := $(EMSDK)/upstream/emscripten/emcc
RANLIB           := $(EMSDK)/upstream/emscripten/emranlib
ARCH             := wasm
BINEXT           := .js
SHLIBEXT         := wasm

DEBUG            := 0
EMCC_DEBUG       := 1

HAVE_VM_COMPILED := true
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
BUILD_CLIENT     := 1

CLIENT_SYSTEM  := sys_common.js \
									sys_browser.js \
									sys_net.js \
									sys_files.js \
									sys_input.js \
									sys_main.js

CLIENT_LIBS    := -lbrowser.js \
								  -lasync.js \
								  -lidbfs.js \
								  -lsdl.js \
									$(addprefix --js-library $(MOUNT_DIR)/wasm/,$(notdir $(CLIENT_SYSTEM))) \
									--js-library $(MOUNT_DIR)/qcommon/vm_js.js \
									--pre-js $(MOUNT_DIR)/wasm/sys_polyfill.js \
									--post-js $(MOUNT_DIR)/wasm/sys_overrides.js

BASE_CFLAGS    += \
				          -Wall -Wno-unused-variable -fno-strict-aliasing \
				          -Wimplicit -Wstrict-prototypes \
				          -DGL_GLEXT_PROTOTYPES=1 -DGL_ARB_ES2_compatibility=1\
				          -DGL_EXT_direct_state_access=1 \
				          -DUSE_Q3KEY -DUSE_MD5 -DEMSCRIPTEN

DEBUG_CFLAGS   := $(BASE_CFLAGS) \
									-O3 -Oz \
								  -frtti \
								  -flto \
									-fPIC \
								  --em-config $(EMJS_CONFIG_PATH)
					
RELEASE_CFLAGS := $(BASE_CFLAGS) \
								  -DNDEBUG \
								  -O3 -Oz \
								  -flto \
									-fPIC \
									--em-config $(EMJS_CONFIG_PATH)

SHLIBCFLAGS    += \
				          -DEMSCRIPTEN \
				          -s STRICT=1 \
									--em-config $(EMJS_CONFIG_PATH)

SHLIBLDFLAGS   += \
									-s MODULARIZE=1 \
									-s STRICT=1 \
									-s EXPORTED_FUNCTIONS="['_GetBotLibAPI']" \
									-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
									-s MODULARIZE=1 \
									-s SIDE_MODULE=1 \
									-s RELOCATABLE=1 \
				          -s LINKABLE=1 \
									--em-config $(EMJS_CONFIG_PATH)

# debug optimize flags: --closure 0 --minify 0 -g -g4 || -O1 --closure 0 --minify 0 -g -g3
# -DDEBUG -D_DEBUG
#  --llvm-lto 3
#   -s USE_WEBGL2=1
#   -s MIN_WEBGL_VERSION=2
#   -s MAX_WEBGL_VERSION=2
#   -s USE_SDL_IMAGE=2 \
#   -s SDL2_IMAGE_FORMATS='["bmp","png","xpm"]' \
#    -s INITIAL_MEMORY=56MB \
			-s GL_UNSAFE_OPTS=0 \
			-s USE_VORBIS=1 \
			-s USE_OGG=1 \
			-s USE_PTHREADS=0 


CLIENT_LDFLAGS += \
									$(CLIENT_LIBS) \
									--em-config $(EMJS_CONFIG_PATH) \
									-s INITIAL_MEMORY=56MB \
									-s MAIN_MODULE=0 \
									-s RELOCATABLE=0 \
				          -s STRICT=1 \
				          -s AUTO_JS_LIBRARIES=0 \
									-s ALLOW_TABLE_GROWTH=1 \
				          --memory-init-file 0 \
									-s USE_SDL=2 \
									-s USE_SDL_MIXER=2 \
				          -s DISABLE_EXCEPTION_CATCHING=0 \
				          -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 \
				          -s ERROR_ON_UNDEFINED_SYMBOLS=1 \
				          -s INVOKE_RUN=1 \
				          -s EXIT_RUNTIME=1 \
				          -s EXTRA_EXPORTED_RUNTIME_METHODS="['FS', 'SYS', 'SYSC',  \
										'SYSF', 'SYSN', 'SYSM', 'ccall', 'callMain', 'addFunction', 'dynCall']" \
				          -s EXPORTED_FUNCTIONS="['_main', '_malloc', '_free', '_atof', \
										'_strncpy', '_memset', '_memcpy', '_fopen', '_fseek', \
										'_Com_WriteConfigToFile', '_IN_PushInit', '_IN_PushEvent', \
										'_S_DisableSounds', '_CL_GetClientState', '_Com_Printf', \
										'_CL_Outside_NextDownload', '_NET_SendLoopPacket', '_SOCKS_Frame_Proxy', \
										'_Com_Frame_Proxy', '_Com_Outside_Error', '_Z_Malloc', '_Z_Free', \
										'_Cvar_Set', '_Cvar_SetValue', '_Cvar_Get', '_Cvar_VariableString', \
										'_Cvar_VariableIntegerValue', '_Cbuf_ExecuteText', '_Cbuf_Execute', \
										'_Cbuf_AddText', '_Field_CharEvent']" \
				          -s FORCE_FILESYSTEM=1 \
								  -s SDL2_IMAGE_FORMATS='[]' \
									-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE="['GetBotLibAPI']" \
									-s INCLUDE_FULL_LIBRARY=0

ifeq ($(USE_RENDERER_DLOPEN),1)
# CLIENT_LDFLAGS += \
					-s EXPORT_ALL=1 \
					-s DECLARE_ASM_MODULE_EXPORTS=1 \
					-s LINKABLE=1 \
					-s INCLUDE_FULL_LIBRARY=1
endif

ifeq ($(BUILD_RENDERER_OPENGL2),1)
# CLIENT_LDFLAGS += \
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

ifeq ($(DEBUG), 1)
CLIENT_LDFLAGS += \
          -s WASM=1 \
          -s MODULARIZE=0 \
          -s SAFE_HEAP=1 \
          -s DEMANGLE_SUPPORT=1 \
          -s ASSERTIONS=2 \
          -s SINGLE_FILE=1
else
CLIENT_LDFLAGS += \
          -s WASM=1 \
          -s MODULARIZE=0 \
          -s SAFE_HEAP=0 \
          -s DEMANGLE_SUPPORT=0 \
          -s ASSERTIONS=0 \
          -s SINGLE_FILE=1
endif

ifdef B
pre-build:
	@echo "" > $(EMJS_CONFIG_PATH)
	@echo "LLVM_ROOT         = '$(EMSDK)/upstream/bin';\\n"               >> $(EMJS_CONFIG_PATH)
	@echo "NODE_JS           = '$(EMSDK)/node/12.9.1_64bit/bin/node';\\n" >> $(EMJS_CONFIG_PATH)
	@echo "BINARYEN_ROOT     = '$(EMSDK)/upstream';\\n"                   >> $(EMJS_CONFIG_PATH)
	@echo "EMSCRIPTEN_ROOT   = '$(EMSDK)/upstream/emscripten';"           >> $(EMJS_CONFIG_PATH)
endif
