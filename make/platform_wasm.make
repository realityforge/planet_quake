HAVE_VM_COMPILED := true
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
USE_SYSTEM_JPEG  := 0
USE_SYSTEM_LIBC  := 0
USE_VID_FAST     := 1
USE_ABS_MOUSE    := 1
USE_LOCAL_DED    := 1
USE_LAZY_LOAD    := 1
USE_LAZY_MEMORY  := 1
USE_MASTER_LAN   := 1

# always build client because we haven't figured out wasm dylinks yet
BUILD_CLIENT     := 1
USE_CURL         := 0
export LDFLAGS="-L/usr/local/opt/llvm/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include"

include make/configure.make

LD               := libs/$(COMPILE_PLATFORM)/wasi-sdk-12.0/bin/wasm-ld
CC               := libs/$(COMPILE_PLATFORM)/wasi-sdk-12.0/bin/clang
CXX              := libs/$(COMPILE_PLATFORM)/wasi-sdk-12.0/bin/clang++
BINEXT           := .wasm

SHLIBEXT         := wasm
SHLIBCFLAGS      := 
#LDFLAGS="-L/usr/local/opt/llvm/lib -rpath,/usr/local/opt/llvm/lib"
LDFLAGS          := --import-memory -error-limit=0

# --entry=_main
CLIENT_LDFLAGS   := code/wasm/include/wasi/libclang_rt.builtins-wasm32.a \
										--export-dynamic --no-entry
SHLIBLDFLAGS     := --unresolved-symbols=ignore-all --export-dynamic --no-entry --strip-all
# -emit-llvm -c -S
BASE_CFLAGS      += -Wall -Ofast --target=wasm32 \
                    -Wimplicit -fstrict-aliasing \
										-Wno-bitwise-op-parentheses \
										-Wno-shift-op-parentheses \
                    -DGL_GLEXT_PROTOTYPES=1 -DGL_ARB_ES2_compatibility=1\
                    -DGL_EXT_direct_state_access=1 \
                    -DUSE_Q3KEY -DUSE_MD5 \
										-DUSE_VID_FAST -DUSE_ABS_MOUSE -DUSE_LOCAL_DED \
										-DUSE_LAZY_LOAD -DUSE_LAZY_MEMORY -DUSE_MASTER_LAN \
                    -fvisibility=hidden \
										-D_XOPEN_SOURCE=700 \
                    -D__WASM__ \
                    --no-standard-libraries \
                    -Icode/wasm/include \
                    -Ilibs/musl-1.2.2/include \
										-Ilibs/musl-1.2.2/arch/generic \

DEBUG_CFLAGS     := $(BASE_CFLAGS) \
                    -std=c11 -DDEBUG -D_DEBUG -frtti -fPIC -O0 -g

RELEASE_CFLAGS   := $(BASE_CFLAGS) \
                    -std=c11 -DNDEBUG -O3 -Oz -flto -fPIC

export INCLUDE   := -Icode/wasm/include \
                    -Ilibs/musl-1.2.2/include \
										-Ilibs/musl-1.2.2/arch/generic


#CLIENT_SYSTEM    := sys_common.js \
                    sys_browser.js \
                    sys_net.js \
                    sys_files.js \
                    sys_input.js \
                    sys_webgl.js \
                    sys_sdl.js \
                    sys_main.js

#SYSEXPORTS := , '_Sys_BeginDownload', '_Sys_SocksConnect', '_Sys_SocksMessage', \
              '_Sys_NET_MulticastLocal', '_Sys_LoadLibrary', '_Sys_DownloadLocalFile', \
              '_Sys_CmdArgsC', '_Sys_CmdArgs'

#LDEXPORTS := '_main', '_malloc', '_free', '_atof', '_sin', '_powf', '_rand', \
  '_strncpy', '_memset', '_memcpy', '_fopen', '_fseek', '_dlopen', '_vsprintf', \
  '_IN_PushInit', '_IN_PushEvent', '_sscanf', '_Cvar_VariableString', \
  '_S_DisableSounds', '_CL_GetClientState', '_Com_Printf', '_strstr', '_strcpy', \
  '_CL_Outside_NextDownload', '_NET_SendLoopPacket', '_SOCKS_Frame_Proxy', \
  '_Com_Frame_Proxy', '_Com_Outside_Error', '_Z_Free', '_Cbuf_Execute', \
  '_Cvar_Set', '_Cvar_SetValue', '_Cvar_Get', '_Cbuf_ExecuteText', \
  '_Cvar_VariableIntegerValue', '_Cbuf_AddText', '_JS_Field_CharEvent', \
  '_FS_AllowedExtension' $(SYSEXPORTS)


ifdef B
pre-build:
	@:
endif
