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
										-Icode/wasm/include \
										-Ilibs/musl-1.2.2/include \
										-Ilibs/musl-1.2.2/arch/generic \
										-Ilibs/musl-1.2.2/arch/wasm

# -emit-llvm -c -S 
MUSL_CFLAGS      := -Wall -Ofast --target=wasm32 -fvisibility=hidden \
										-D_XOPEN_SOURCE=600 -D_ALL_SOURCE=700 \
										-D__WASM__ \
										-fno-common -std=c99 -ffreestanding -nostdinc -pedantic \
										-Wno-unused-variable -Wvariadic-macros -Wno-extra-semi \
										-Wno-shift-op-parentheses -Wno-c11-extensions \
										-Wno-dollar-in-identifier-extension -Wno-unused-function \
										-Wno-incompatible-pointer-types -Wno-logical-op-parentheses \
										-Wno-bitwise-op-parentheses -Wno-int-conversion \
										-Wno-tautological-constant-out-of-range-compare \
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

WORKDIRS         += musl          musl/string musl/stdio  musl/stdlib  \
										musl/signal   musl/errno  musl/math   musl/unistd  \
										musl/internal musl/time   musl/locale musl/network \
										musl/select   musl/stat   musl/dirent musl/misc    \
										musl/fcntl    musl/ctype  musl/exit   musl/env     \
										musl/thread   musl/multibyte
CLEANS           += musl $(CNAME)$(ARCHEXT).bc $(CNAME)$(ARCHEXT).o
MUSL_LOBJ        := string/stpcpy.o  string/memset.o  string/memcpy.o \
										string/memmove.o string/memcmp.o  string/memchr.o \
										string/memrchr.o string/strncpy.o string/strcmp.o \
										string/strcat.o  string/strchr.o  string/strncmp.o \
										string/strcpy.o  string/stpncpy.o string/strchrnul.o \
										string/strlen.o  string/strncat.o \
										string/strstr.o  string/strrchr.o string/strnlen.o \
										string/strcspn.o string/strpbrk.o \
										\
										internal/shgetc.o internal/syscall_ret.o \
										\
										stdio/sprintf.o  stdio/fprintf.o stdio/vsnprintf.o \
										stdio/vfprintf.o stdio/fwrite.o  stdio/sscanf.o \
										stdio/vsscanf.o  stdio/vfscanf.o stdio/snprintf.o \
										stdio/vsprintf.o stdio/setvbuf.o stdio/fread.o \
										stdio/ftell.o    stdio/fflush.o  stdio/fopen.o \
										stdio/fputs.o    stdio/stdout.o  stdio/stderr.o \
										stdio/fseek.o    stdio/fclose.o  stdio/remove.o \
										stdio/rename.o   stdio/pclose.o  stdio/popen.o \
										stdio/__lockfile.o \
										\
										stdlib/atoi.o stdlib/atof.o stdlib/strtod.o stdlib/strtol.o \
										ctype/tolower.o \
										\
										signal/signal.o signal/sigaddset.o signal/sigemptyset.o \
										errno/strerror.o errno/__errno_location.o \
										\
										math/__signbit.o    math/__signbitf.o    math/__signbitl.o \
										math/__fpclassify.o math/__fpclassifyf.o math/__fpclassifyl.o \
										\
										\
										unistd/getpid.o unistd/getcwd.o unistd/readlink.o \
										unistd/read.o unistd/write.o unistd/close.o \
										\
										exit/assert.o \
										\
										time/ctime.o time/time.o time/localtime.o time/asctime.o \
										time/asctime_r.o time/gettimeofday.o time/clock.o \
										time/localtime_r.o time/clock_gettime.o \
										\
										locale/c_locale.o \
										\
										network/gethostbyname2.o network/gethostbyname.o network/htons.o \
										network/ntohs.o  network/getaddrinfo.o network/freeaddrinfo.o \
										network/socket.o network/connect.o     network/recvfrom.o \
										network/recv.o   network/send.o        network/sendto.o \
										network/getnameinfo.o \
										\
										select/select.o \
										\
										stat/umask.o stat/mkdir.o stat/stat.o stat/fstatat.o \
										\
										dirent/readdir.o dirent/closedir.o dirent/opendir.o \
										misc/dirname.o \
										\
										env/getenv.o \
										fcntl/fcntl.o fcntl/open.o \
										thread/__lock.o \
										multibyte/mbrtowc.o \
										

LIBOBJ           += $(addprefix $(B)/musl/,$(MUSL_LOBJ))
										

define DO_MUSL_CC
  $(echo_cmd) "MUSL_CC $<"
  $(Q)$(CC) -o $@ $(MUSL_CFLAGS) -c $<
endef

ifdef B
$(B)/musl/%.o: libs/musl-1.2.2/src/%.c
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
