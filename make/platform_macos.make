
include make/configure.make

CC               := gcc
CXX              := /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
GXX              := g++

BASE_CFLAGS      += -Wall -fno-strict-aliasing -Wimplicit \
                    -Wstrict-prototypes -pipe \
                    -Wno-unused-result
OPTIMIZE         := -O2 -fvisibility=hidden
SHLIBEXT         := dylib
SHLIBCFLAGS      := -fPIC -fvisibility=hidden -fno-common
LDFLAGS          :=
SHLIBLDFLAGS     := -dynamiclib $(LDFLAGS)

ifneq ($(SDL_INCLUDE),)
  BASE_CFLAGS    += $(SDL_INCLUDE)
  CLIENT_LDFLAGS += $(SDL_LIBS)
else
  BASE_CFLAGS    += -I/Library/Frameworks/SDL2.framework/Headers
  CLIENT_LDFLAGS += -F/Library/Frameworks -framework SDL2
endif
ifeq ($(USE_SYSTEM_JPEG),1)
CLIENT_LDFLAGS   += $(JPEG_LIBS)
endif

ifeq ($(USE_RMLUI),1)
ifeq ($(USE_RMLUI_DLOPEN),0)
ifeq ($(USE_SYSTEM_RMLUI),1)
  CLIENT_LDFLAGS += -L$(B) -lRmlCore_$(ARCH)
else
  CLIENT_LDFLAGS += $(B)/libRmlCore_$(SHLIBNAME)
endif
endif
endif

ifeq ($(USE_CIN_VPX),1)
BASE_CFLAGS    += -Ilibs/libvpx-1.10
CLIENT_LDFLAGS += $(VPX_LIBS) $(VORBIS_LIBS) $(OPUS_LIBS)
ifeq ($(USE_SYSTEM_VPX),1)
else
CLIENT_LDFLAGS += $(B)/libwebm_$(SHLIBNAME)
endif
endif

BASE_CFLAGS      += -I/usr/include -I/usr/local/include -isysroot $(SYSROOT)
DEBUG_CFLAGS      = $(BASE_CFLAGS) -DDEBUG -D_DEBUG -g -O0
RELEASE_CFLAGS    = $(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)
USE_SDL           = 1

ifdef B
pre-build:
	@:
endif
