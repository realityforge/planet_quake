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
                    stdio/sprintf.o  stdio/fprintf.o   stdio/vsnprintf.o \
                    stdio/vfprintf.o stdio/sscanf.o    stdio/fwrite.o    \
                    stdio/vsscanf.o  stdio/vfscanf.o   stdio/snprintf.o  \
                    stdio/vsprintf.o stdio/setvbuf.o   \
                    stdio/__toread.o stdio/__towrite.o \
                    stdio/__lockfile.o stdio/__uflow.o          \
                    \
                    internal/shgetc.o internal/floatscan.o internal/intscan.o \
                    \
                    errno/strerror.o errno/__errno_location.o locale/__lctrans.o \
                    \
                    math/__signbit.o    math/__signbitf.o    math/__signbitl.o \
                    math/__fpclassify.o math/__fpclassifyf.o math/__fpclassifyl.o \
                    math/frexpl.o       math/scalbn.o        math/copysignl.o \
										math/scalbnl.o      math/fmodl.o \
                    math/__math_oflowf.o math/__math_uflowf.o \
                    math/__math_invalidf.o math/__math_xflowf.o \
                    \
                    multibyte/mbrtowc.o   multibyte/mbstowcs.o multibyte/wctomb.o \
                    multibyte/mbsrtowcs.o multibyte/wcrtomb.o  multibyte/mbsinit.o \
                    multibyte/mbtowc.o    multibyte/btowc.o    multibyte/internal.o \


MUSL_OBJ         += $(addprefix $(B)/musl/,$(MUSL_LOBJ))

MUSL_CFLAGS      :=
ifeq ($(BUILD_CLIENT),1)
MUSL_CFLAGS      += $(CLIENT_CFLAGS)
endif
MUSL_CFLAGS      += -D_XOPEN_SOURCE=700 \
										-std=gnu11 \
                    -Wno-unused-variable -Wvariadic-macros -Wno-extra-semi \
                    -Wno-shift-op-parentheses -Wno-c11-extensions \
                    -Wno-dollar-in-identifier-extension -Wno-unused-function \
                    -Wno-incompatible-pointer-types -Wno-logical-op-parentheses \
                    -Wno-bitwise-op-parentheses -Wno-int-conversion \
                    -Wno-tautological-constant-out-of-range-compare \
                    -Wno-string-plus-int -Wno-unsupported-visibility \
										-Wno-strict-prototypes  \
                    -Wno-bitwise-op-parentheses -Wno-gnu-include-next \
                    -Wno-unknown-attributes -Wno-ignored-attributes \
                    

export MUSL_INCLUDE := -Ilibs/musl-1.2.2/arch/generic \
                       -Ilibs/musl-1.2.2/src/internal \
                       -Ilibs/musl-1.2.2/src/include

define DO_MUSL_CC
	$(echo_cmd) "MUSL_CC $<"
	$(Q)$(CC) -o $@ $(MUSL_INCLUDE) $(MUSL_CFLAGS) -c $<
endef

ifdef B
$(B)/musl/%.o: $(MUSL_SOURCE)/src/%.c
	$(DO_MUSL_CC)

$(B)/$(MUSL_TARGET): $(MUSL_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(MUSL_Q3OBJ) $(MUSL_CFLAGS) $(CLIENT_LDFLAGS)
endif
