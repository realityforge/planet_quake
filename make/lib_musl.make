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


MUSL_LOBJ        := string/memset.o  string/memcpy.o    \
                    string/memmove.o string/memcmp.o  string/memchr.o    \
                    string/memrchr.o string/strncpy.o string/strcmp.o    \
                    string/strcat.o  string/strchr.o  string/strncmp.o   \
                    string/strcpy.o  string/stpncpy.o string/strchrnul.o \
                    string/strlen.o  string/strncat.o string/strspn.o    \
                    string/strstr.o  string/strrchr.o string/strnlen.o   \
                    string/strcspn.o string/strpbrk.o string/strdup.o    \
                    \
                    stdlib/qsort.o  \
                    \
                    errno/__errno_location.o \
                    \
                    network/ntohs.o  network/htons.o



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
MUSL_INCLUDE     := -Ilibs/musl-1.2.2/arch/generic \
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
