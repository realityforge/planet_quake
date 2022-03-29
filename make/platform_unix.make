include make/configure.make

BASE_CFLAGS += -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

BASE_CFLAGS += -Wno-unused-result

BASE_CFLAGS += -DUSE_ICON

BASE_CFLAGS += -I/usr/include -I/usr/local/include

OPTIMIZE = -O2 -fvisibility=hidden

ifeq ($(ARCH),x86_64)
  ARCHEXT = .x64
else
ifeq ($(ARCH),x86)
  OPTIMIZE += -march=i586 -mtune=i686
endif
endif

ifeq ($(ARCH),arm)
  OPTIMIZE += -march=armv7-a
  ARCHEXT = .arm
endif

ifeq ($(ARCH),aarch64)
  ARCHEXT = .aarch64
endif

SHLIBEXT = so
SHLIBCFLAGS = -fPIC -fvisibility=hidden -fno-common
SHLIBLDFLAGS = -shared $(LDFLAGS)

CLIENT_LDFLAGS += -lm

ifeq ($(USE_SDL),1)
  BASE_CFLAGS += $(SDL_CFLAGS) -I/usr/include/SDL2
  CLIENT_LDFLAGS += -lSDL2
else
  BASE_CFLAGS += $(X11_CFLAGS) -I/usr/X11R6/include
  CLIENT_LDFLAGS += -lX11
endif

ifeq ($(USE_CODEC_VORBIS),1)
  CLIENT_LDFLAGS += -lvorbisfile -lvorbis -logg
endif

ifeq ($(USE_SYSTEM_ZLIB),1)
  BASE_CFLAGS    += -Ilibs/zlib/contrib/minizip -DUSE_SYSTEM_ZLIB
  CLIENT_LDFLAGS += $(ZLIB_LIBS)
endif

ifeq ($(USE_SYSTEM_JPEG),1)
  CLIENT_LDFLAGS += $(JPEG_LIBS)
endif

ifeq ($(USE_CURL),1)
  ifeq ($(USE_CURL_DLOPEN),0)
    CLIENT_LDFLAGS += -lcurl
  endif
endif

ifeq ($(PLATFORM),linux)
  CLIENT_LDFLAGS += -ldl -Wl,--hash-style=both
  ifeq ($(ARCH),x86)
    BASE_CFLAGS += -m32
    CLIENT_LDFLAGS += -m32
  endif
endif

DEBUG_CFLAGS = -DDEBUG -D_DEBUG -g -O0
RELEASE_CFLAGS = -DNDEBUG $(OPTIMIZE)
