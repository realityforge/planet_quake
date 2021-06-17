HAVE_VM_COMPILED := true
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
# always build client because we haven't figured out dylinks yet
BUILD_CLIENT     := 1
USE_CURL         := 0
USE_SYSTEM_JPEG  := 0
export LDFLAGS="-L/usr/local/opt/llvm/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include"

include make/configure.make

LD               := /usr/local/opt/llvm/bin/wasm-ld
CC               := /usr/local/opt/llvm/bin/clang
CXX              := /usr/local/opt/llvm/bin/clang++
BINEXT           := .wasm

SHLIBEXT         := wasm
SHLIBCFLAGS      := 
#LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib"
LDFLAGS          := --import-memory
# --entry=_main
CLIENT_LDFLAGS   := --warn-unresolved-symbols --export-dynamic --no-entry --export-all
SHLIBLDFLAGS     := --unresolved-symbols=ignore-all --export-dynamic --no-entry --strip-all
# -emit-llvm -c -S
BASE_CFLAGS      += -Wall -Ofast --target=wasm32 \
										-Wno-unused-variable \
                    -Wimplicit -Wstrict-prototypes -fno-strict-aliasing \
                    -DGL_GLEXT_PROTOTYPES=1 -DGL_ARB_ES2_compatibility=1\
                    -DGL_EXT_direct_state_access=1 \
                    -DUSE_Q3KEY -DUSE_MD5 -D__WASM__ \
										-fvisibility=hidden \
										-D_XOPEN_SOURCE=600 \
										-D_ALL_SOURCE=700 \
										-Ilibs/musl-1.2.2/include \
										-Ilibs/musl-1.2.2/arch/generic \
										-Ilibs/musl-1.2.2/arch/wasm \
										-Ilibs/emsdk/upstream/emscripten/system/include

# -emit-llvm -c -S 
MUSL_CFLAGS      := -Wall -Ofast --target=wasm32 -fvisibility=hidden \
										-D_XOPEN_SOURCE=600 -D_ALL_SOURCE=700 \
										-fno-common -std=c99 -ffreestanding -nostdinc -pedantic \
										-Wno-unused-variable -Wvariadic-macros -Wno-extra-semi \
										-Wno-shift-op-parentheses \
										-Ilibs/musl-1.2.2/arch/generic \
										-Ilibs/musl-1.2.2/arch/wasm \
										-Ilibs/musl-1.2.2/src/include \
										-Ilibs/musl-1.2.2/src/internal \
										-Ilibs/musl-1.2.2/include

DEBUG_CFLAGS     := $(BASE_CFLAGS) \
                    -std=c11 -DDEBUG -D_DEBUG -frtti -fPIC -O0 -g

RELEASE_CFLAGS   := $(BASE_CFLAGS) \
                    -std=c11 -DNDEBUG -O3 -Oz -flto -fPIC

export INCLUDE	 := -Ilibs/musl-1.2.2/include \
										-Ilibs/musl-1.2.2/arch/generic \
										-Ilibs/musl-1.2.2/arch/wasm

WORKDIRS         += musl
CLEANS           += musl $(CNAME)$(ARCHEXT).bc $(CNAME)$(ARCHEXT).o
LOBJ             := memset.o memcpy.o memmove.o memcmp.o memchr.o \
										strncpy.o strcmp.o strcat.o strchr.o sprintf.o strncmp.o \
										strcpy.o stpncpy.o strchrnul.o strlen.o strncat.o strtod.o \
										fprintf.o vsnprintf.o vfprintf.o atoi.o atof.o strstr.o \
										tolower.o gethostbyname.o strrchr.o strnlen.o

LIBOBJ           += $(addprefix $(B)/musl/,$(notdir $(LOBJ)))
										

define DO_MUSL_CC
  $(echo_cmd) "MUSL_CC $<"
  $(Q)$(CC) -o $@ $(MUSL_CFLAGS) -c $<
endef

ifdef B
$(B)/musl/%.o: libs/musl-1.2.2/src/ctype/%.c
	$(DO_MUSL_CC)

$(B)/musl/%.o: libs/musl-1.2.2/src/stdlib/%.c
	$(DO_MUSL_CC)

$(B)/musl/%.o: libs/musl-1.2.2/src/network/%.c
	$(DO_MUSL_CC)

$(B)/musl/%.o: libs/musl-1.2.2/src/stdio/%.c
	$(DO_MUSL_CC)

$(B)/musl/%.o: libs/musl-1.2.2/src/string/%.c
	$(DO_MUSL_CC)
endif

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
