MUSL_WORKDIR     := libmusl
MUSL_SOURCE      := libs/musl-1.2.2

BUILD_LIBMUSL    := 1
ifneq ($(BUILD_CLIENT),1)
MKFILE           := $(lastword $(MAKEFILE_LIST))
include make/platform.make
endif

MUSL_TARGET      := $(CNAME)_libmusl_$(SHLIBNAME)
MUSL_SOURCES     := 
MUSL_INCLUDES    := 
MUSL_LIBS        := 

ifeq ($(BUILD_CLIENT),1)
WORKDIRS         += musl          musl/string musl/stdio  musl/stdlib    \
                    musl/signal   musl/errno  musl/math   musl/unistd    \
                    musl/internal musl/time   musl/locale musl/network   \
                    musl/select   musl/stat   musl/dirent musl/misc      \
                    musl/fcntl    musl/ctype  musl/exit   musl/env       \
                    musl/thread   musl/mman   musl/process \
                    musl/multibyte musl/malloc

CLEANS           += musl $(CNAME)$(ARCHEXT).bc $(CNAME)$(ARCHEXT).o
endif

#                malloc/lite_malloc.o malloc/free.o malloc/calloc.o malloc/realloc.o \


MUSL_LOBJ        := string/stpcpy.o  string/memset.o  string/memcpy.o    \
                    string/memmove.o string/memcmp.o  string/memchr.o    \
                    string/memrchr.o string/strncpy.o string/strcmp.o    \
                    string/strcat.o  string/strchr.o  string/strncmp.o   \
                    string/strcpy.o  string/stpncpy.o string/strchrnul.o \
                    string/strlen.o  string/strncat.o string/strspn.o    \
                    string/strstr.o  string/strrchr.o string/strnlen.o   \
                    string/strcspn.o string/strpbrk.o string/strdup.o    \
                    \
                        internal/syscall_ret.o \
                     internal/procfdname.o internal/libc.o \
                    \
                     \
                    \
                    \
                     \
                    stdlib/qsort.o  \
                    \
                    ctype/tolower.o ctype/isalnum.o  ctype/isspace.o \
                    ctype/isdigit.o ctype/iswdigit.o ctype/isupper.o \
										ctype/isalpha.o \
                    \
                    errno/strerror.o errno/__errno_location.o \
                    \
                    \
                    math/__signbit.o    math/__signbitf.o    math/__signbitl.o \
                    math/__fpclassify.o math/__fpclassifyf.o math/__fpclassifyl.o \
                    math/frexpl.o       math/scalbn.o        math/copysignl.o \
										math/scalbnl.o      math/fmodl.o         math/fabsl.o \
                             math/__math_oflowf.o math/__math_uflowf.o \
                    math/__math_invalidf.o math/__math_xflowf.o \
                    \
                    unistd/getpid.o unistd/getcwd.o unistd/readlink.o \
                    unistd/read.o   unistd/write.o  unistd/close.o \
                    unistd/lseek.o  unistd/pipe2.o  unistd/_exit.o \
                    unistd/gethostname.o unistd/pipe.o \
                    \
                    \
                    time/ctime.o       time/time.o           time/localtime.o \
                     time/gettimeofday.o   time/clock.o \
                    time/localtime_r.o time/clock_gettime.o  time/__tz.o \
                      time/__year_to_secs.o time/__map_file.o \
                    time/__month_to_secs.o time/__secs_to_tm.o \
                    \
                    locale/c_locale.o locale/__lctrans.o locale/langinfo.o \
                    \
                    network/ntohs.o  network/htons.o \
                    \
                    select/select.o select/poll.o \
                    \
                    stat/umask.o stat/mkdir.o stat/stat.o stat/fstatat.o \
                    \
                    dirent/readdir.o dirent/closedir.o dirent/opendir.o \
                    \
                    misc/dirname.o misc/ioctl.o misc/uname.o \
                    \
                    mman/munmap.o mman/mmap.o \
                    \
                    env/getenv.o env/__environ.o \
                    \
                    fcntl/fcntl.o fcntl/open.o \
                    \
                    thread/__lock.o       thread/pthread_sigmask.o \
                    thread/__syscall_cp.o thread/pthread_setcancelstate.o \
                    thread/pthread_cleanup_push.o thread/clone.o \


MUSL_OBJ         += $(addprefix $(B)/musl/,$(MUSL_LOBJ))
MUSL_CFLAGS      := -Ofast --target=wasm32 -fvisibility=hidden \
                    -D_XOPEN_SOURCE=700 \
                    -D__WASM__ \
										-fno-common -ffreestanding -nostdinc -pedantic \
										--no-standard-libraries \
										-std=gnu11 \
										-Wall \
                    -Wno-unused-variable -Wvariadic-macros -Wno-extra-semi \
                    -Wno-shift-op-parentheses -Wno-c11-extensions \
                    -Wno-dollar-in-identifier-extension -Wno-unused-function \
                    -Wno-incompatible-pointer-types -Wno-logical-op-parentheses \
                    -Wno-bitwise-op-parentheses -Wno-int-conversion \
                    -Wno-tautological-constant-out-of-range-compare \
                    -Wno-string-plus-int -Wno-unsupported-visibility \
										-Wno-shift-op-parentheses -Wno-strict-prototypes  \
                    -Wno-bitwise-op-parentheses -Wno-gnu-include-next \
                    -Wno-unknown-attributes -Wno-ignored-attributes \
										-Icode/wasm/include \
                    -Ilibs/musl-1.2.2/arch/generic \
                    -Ilibs/musl-1.2.2/src/include \
                    -Ilibs/musl-1.2.2/src/internal

#DEBUG_CFLAGS     := $(BASE_CFLAGS) \
                    -std=c11 -DDEBUG -D_DEBUG -frtti -fPIC -O0 -g

#RELEASE_CFLAGS   := $(BASE_CFLAGS) \
                    -std=c11 -DNDEBUG -O3 -Oz -flto -fPIC
MUSL_INCLUDE     := -Icode/wasm/include \
										-Ilibs/musl-1.2.2/arch/generic \
                    -Ilibs/musl-1.2.2/src/include \
                    -Ilibs/musl-1.2.2/src/internal

export MUSL_INCLUDE

define DO_MUSL_CC
	$(echo_cmd) "MUSL_CC $<"
	$(Q)$(CC) -o $@ $(MUSL_INCLUDE) $(CFLAGS) $(MUSL_CFLAGS) -c $<
endef

ifdef B
$(B)/musl/%.o: $(MUSL_SOURCE)/src/%.c
	$(DO_MUSL_CC)

$(B)/$(MUSL_TARGET): $(MUSL_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(MUSL_Q3OBJ) $(CFLAGS) $(CLIENT_LDFLAGS)
endif
