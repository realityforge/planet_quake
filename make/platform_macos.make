CC       := gcc
CXX      := /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
GXX      := g++

BASE_CFLAGS += -Wall -fno-strict-aliasing -Wimplicit \
               -Wstrict-prototypes -pipe \
               -Wno-unused-result

OPTIMIZE = -O2 -fvisibility=hidden

SHLIBEXT = dylib
SHLIBCFLAGS = -fPIC -fvisibility=hidden -fno-common
SHLIBLDFLAGS = -dynamiclib $(LDFLAGS)

LDFLAGS =

ifneq ($(SDL_INCLUDE),)
  BASE_CFLAGS += $(SDL_INCLUDE)
  CLIENT_LDFLAGS = $(SDL_LIBS)
else
  BASE_CFLAGS += -I/Library/Frameworks/SDL2.framework/Headers
  CLIENT_LDFLAGS =  -F/Library/Frameworks -framework SDL2
endif
BASE_CFLAGS   += -I/usr/include -I/usr/local/include

DEBUG_CFLAGS   = $(BASE_CFLAGS) -DDEBUG -D_DEBUG -g -O0
RELEASE_CFLAGS = $(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)
USE_SDL        = 1

ifdef B
pre-build:
	@:
endif
