USE_SDL=0

include make/configure.make

ifeq ($(CROSS_COMPILING),1)
  # If CC is already set to something generic, we probably want to use
  # something more specific
  ifneq ($(findstring $(strip $(CC)),cc gcc),)
    CC=
  endif

  # We need to figure out the correct gcc and windres
  ifeq ($(ARCH),x86_64)
    MINGW_PREFIXES=x86_64-w64-mingw32 amd64-mingw32msvc
    STRIP=x86_64-w64-mingw32-strip
  endif
  ifeq ($(ARCH),x86)
    MINGW_PREFIXES=i686-w64-mingw32 i586-mingw32msvc i686-pc-mingw32
  endif

  ifndef CC
    CC=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
       $(call bin_path, $(MINGW_PREFIX)-gcc))))
  endif

#   STRIP=$(MINGW_PREFIX)-strip -g

  ifndef WINDRES
    WINDRES=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
       $(call bin_path, $(MINGW_PREFIX)-windres))))
  endif
else
  # Some MinGW installations define CC to cc, but don't actually provide cc,
  # so check that CC points to a real binary and use gcc if it doesn't
  ifeq ($(call bin_path, $(CC)),)
    CC=gcc
  endif

endif

# using generic windres if specific one is not present
ifeq ($(WINDRES),)
  WINDRES=windres
endif

ifeq ($(CC),)
  $(error Cannot find a suitable cross compiler for $(PLATFORM))
endif

BASE_CFLAGS += -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
  -DUSE_ICON -DMINGW=1

BASE_CFLAGS += -Wno-unused-result -I/usr/include/glib-2.0

ifeq ($(ARCH),x86_64)
  ARCHEXT = .x64
  BASE_CFLAGS += -m64
  OPTIMIZE = -O2 -ffast-math -fstrength-reduce
endif
ifeq ($(ARCH),x86)
  BASE_CFLAGS += -m32
  OPTIMIZE = -O2 -march=i586 -mtune=i686 -ffast-math -fstrength-reduce
endif

SHLIBEXT = dll
SHLIBCFLAGS = -fPIC -fvisibility=hidden -fno-common  -I/usr/include/glib-2.0
SHLIBLDFLAGS = -shared $(LDFLAGS)

BINEXT = .exe

LDFLAGS = -mwindows -Wl,--dynamicbase -Wl,--nxcompat  -fvisibility=hidden
LDFLAGS += -lwsock32 -lgdi32 -lwinmm -lole32 -lws2_32 -lpsapi -lcomctl32

CLIENT_LDFLAGS=$(LDFLAGS)

ifeq ($(USE_SDL),1)
  BASE_CFLAGS += -DUSE_LOCAL_HEADERS=1 -I$(MOUNT_DIR)/libsdl/windows/include/SDL2
  #CLIENT_CFLAGS += -DUSE_LOCAL_HEADERS=1
  ifeq ($(ARCH),x86)
    CLIENT_LDFLAGS += -L$(MOUNT_DIR)/libsdl/windows/mingw/lib32
    CLIENT_LDFLAGS += -lSDL2
    CLIENT_EXTRA_FILES += $(MOUNT_DIR)/libsdl/windows/mingw/lib32/SDL2.dll
  else
    CLIENT_LDFLAGS += -L$(MOUNT_DIR)/libsdl/windows/mingw/lib64
    CLIENT_LDFLAGS += -lSDL264
    CLIENT_EXTRA_FILES += $(MOUNT_DIR)/libsdl/windows/mingw/lib64/SDL264.dll
  endif
endif

ifeq ($(USE_CODEC_VORBIS),1)
  CLIENT_LDFLAGS += -lvorbisfile -lvorbis -logg
endif

ifeq ($(USE_CURL),1)
  BASE_CFLAGS += -I$(MOUNT_DIR)/libcurl/windows/include
  ifeq ($(ARCH),x86)
    CLIENT_LDFLAGS += -L$(MOUNT_DIR)/libcurl/windows/mingw/lib32
  else
    CLIENT_LDFLAGS += -L$(MOUNT_DIR)/libcurl/windows/mingw/lib64
  endif
  CLIENT_LDFLAGS += -lcurl -lwldap32 -lcrypt32
endif

DEBUG_CFLAGS = $(BASE_CFLAGS) -DDEBUG -D_DEBUG -g -O0
RELEASE_CFLAGS = $(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)
