
include make/configure.make

SYSROOT      := $(shell xcrun --show-sdk-path)
#SYSROOT       := /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk

CC               := gcc
CXX              := c++
GXX              := g++

BASE_CFLAGS      += -Wall -fno-strict-aliasing -Wimplicit \
                    -Wstrict-prototypes -pipe \
                    -Wno-unused-result
                    # -fsigned-char -ftree-vectorize 
                    
OPTIMIZE         := -O2 -fvisibility=hidden -ffast-math -fno-short-enums -MMD
SHLIBEXT         := dylib
SHLIBCFLAGS      := -fPIC -fvisibility=hidden -fno-common
SHLIBLDFLAGS     := -dynamiclib -rdynamic

ifneq ($(SDL_CFLAGS),)
  BASE_CFLAGS    += $(SDL_CFLAGS)
  CLIENT_LDFLAGS += $(SDL_LIBS)
else
  BASE_CFLAGS    += -I/Library/Frameworks/SDL2.framework/Headers
  CLIENT_LDFLAGS += -F/Library/Frameworks -framework SDL2
endif
ifeq ($(USE_SYSTEM_JPEG),1)
  CLIENT_LDFLAGS += $(JPEG_LIBS)
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
ifeq ($(USE_SYSTEM_VPX),0)
CLIENT_LDFLAGS += $(B)/libwebm_$(SHLIBNAME)
endif
endif

ifeq ($(USE_SYSTEM_ZLIB),1)
  BASE_CFLAGS    += -Ilibs/zlib/contrib/minizip -DUSE_SYSTEM_ZLIB
  CLIENT_LDFLAGS += $(ZLIB_LIBS)
endif


BASE_CFLAGS      += -I/usr/include -I/usr/local/include -isysroot $(SYSROOT)
DEBUG_CFLAGS      = -DDEBUG -D_DEBUG -g -O0 -g3 -fno-inline
RELEASE_CFLAGS    = -DNDEBUG $(OPTIMIZE)
USE_SDL          ?= 1

