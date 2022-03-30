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
                    internal/shgetc.o    internal/syscall_ret.o internal/intscan.o \
                    internal/floatscan.o internal/procfdname.o internal/libc.o \
                    \
                    stdio/sprintf.o  stdio/fprintf.o   stdio/vsnprintf.o \
                    stdio/vfprintf.o stdio/fwrite.o    stdio/sscanf.o    \
                    stdio/vsscanf.o  stdio/vfscanf.o   stdio/snprintf.o  \
                    stdio/vsprintf.o stdio/setvbuf.o   stdio/fread.o     \
                    stdio/ftell.o    stdio/fflush.o    stdio/fopen.o     \
                    stdio/fputs.o    stdio/stdout.o    stdio/stderr.o    \
                    stdio/fseek.o    stdio/fclose.o    stdio/remove.o    \
                    stdio/rename.o   stdio/pclose.o    stdio/popen.o     \
                    stdio/ofl.o      stdio/fgets.o     stdio/getc.o      \
                    stdio/ofl_add.o  stdio/vfwprintf.o stdio/fputwc.o    \
                    stdio/__lockfile.o    stdio/__fclose_ca.o      \
                    stdio/__fdopen.o      stdio/__stdout_write.o   \
                    stdio/__stdio_close.o stdio/__stdio_seek.o     \
                    stdio/__fopen_rb_ca.o stdio/__fmodeflags.o     \
                    stdio/__stdio_write.o stdio/__toread.o         \
                    stdio/__stdio_read.o  stdio/__towrite.o        \
                    stdio/__stdio_exit.o  stdio/__uflow.o          \
                    \
                    \
                    stdlib/atoi.o   stdlib/atof.o   stdlib/strtod.o \
                    stdlib/qsort.o  stdlib/strtol.o stdlib/atol.o \
                    \
                    ctype/tolower.o ctype/isalnum.o  ctype/isspace.o \
                    ctype/isdigit.o ctype/iswdigit.o ctype/isupper.o \
										ctype/isalpha.o \
                    \
                    signal/signal.o      signal/sigaddset.o   signal/sigemptyset.o \
                    signal/sigprocmask.o signal/sigaction.o   signal/block.o \
										signal/raise.o       signal/sigismember.o signal/restore.o \
                    \
                    errno/strerror.o errno/__errno_location.o \
                    \
										process/posix_spawn_file_actions_init.o \
										process/posix_spawn_file_actions_adddup2.o \
										process/posix_spawn_file_actions_destroy.o \
										process/posix_spawn.o process/waitpid.o \
										process/execve.o \
                    \
                    math/__signbit.o    math/__signbitf.o    math/__signbitl.o \
                    math/__fpclassify.o math/__fpclassifyf.o math/__fpclassifyl.o \
                    math/frexpl.o       math/scalbn.o        math/copysignl.o \
										math/scalbnl.o      math/fmodl.o         math/fabsl.o \
                    math/powf.o         math/__math_oflowf.o math/__math_uflowf.o \
                    math/__math_invalidf.o math/__math_xflowf.o \
                    \
                    unistd/getpid.o unistd/getcwd.o unistd/readlink.o \
                    unistd/read.o   unistd/write.o  unistd/close.o \
                    unistd/lseek.o  unistd/pipe2.o  unistd/_exit.o \
                    unistd/gethostname.o unistd/pipe.o \
                    \
                    exit/assert.o exit/abort_lock.o exit/exit.o exit/_Exit.o \
										exit/abort.o \
                    \
                    time/ctime.o       time/time.o           time/localtime.o \
                    time/asctime_r.o   time/gettimeofday.o   time/clock.o \
                    time/localtime_r.o time/clock_gettime.o  time/__tz.o \
                    time/asctime.o     time/__year_to_secs.o time/__map_file.o \
                    time/__month_to_secs.o time/__secs_to_tm.o \
                    \
                    locale/c_locale.o locale/__lctrans.o locale/langinfo.o \
                    \
                    network/ntohs.o  network/htons.o   network/h_errno.o \
                    network/socket.o network/connect.o network/recvfrom.o \
                    network/recv.o   network/send.o    network/sendto.o \
                    network/bind.o   network/gai_strerror.o \
                    network/getnameinfo.o      network/lookup_name.o \
                    network/gethostbyname2_r.o network/getsockname.o \
                    network/dns_parse.o        network/getifaddrs.o \
                    network/res_mkquery.o      network/res_msend.o \
                    network/resolvconf.o       network/if_indextoname.o \
                    network/inet_ntop.o        network/res_send.o \
                    network/dn_expand.o        network/lookup_ipliteral.o \
                    network/if_nametoindex.o   network/setsockopt.o \
                    network/inet_aton.o        network/inet_pton.o \
                    network/netlink.o          network/lookup_serv.o \
                    network/gethostbyname2.o   network/gethostbyname.o \
                    network/getaddrinfo.o      network/freeaddrinfo.o \
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
                    \
                    multibyte/mbrtowc.o   multibyte/mbstowcs.o multibyte/wctomb.o \
                    multibyte/mbsrtowcs.o multibyte/wcrtomb.o  multibyte/mbsinit.o \
                    multibyte/mbtowc.o    multibyte/btowc.o    multibyte/internal.o \


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

ifeq ($(BUILD_CLIENT),1)
MUSL_CFLAGS      += $(CLIENT_CFLAGS)
endif

#DEBUG_CFLAGS     := $(BASE_CFLAGS) \
                    -std=c11 -DDEBUG -D_DEBUG -frtti -fPIC -O0 -g

#RELEASE_CFLAGS   := $(BASE_CFLAGS) \
                    -std=c11 -DNDEBUG -O3 -Oz -flto -fPIC

define DO_MUSL_CC
	$(echo_cmd) "MUSL_CC $<"
	$(Q)$(CC) -o $@ $(CFLAGS) $(MUSL_CFLAGS) -c $<
endef

ifdef B
$(B)/musl/%.o: $(MUSL_SOURCE)/src/%.c
	$(DO_MUSL_CC)

$(B)/$(MUSL_TARGET): $(MUSL_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(MUSL_Q3OBJ) $(CFLAGS) $(CLIENT_LDFLAGS)
endif
