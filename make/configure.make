
include Makefile.local

CNAME               ?= quake3e
DNAME               ?= quake3e.ded

BUILD_CLIENT        ?= 0
BUILD_SLIM_CLIENT   ?= 0
BUILD_SERVER        ?= 0
BUILD_GAME_STATIC   ?= 0
BUILD_GAMES         ?= 0
BUILD_LIBS          ?= 0
BUILD_GAME_QVM      ?= 0
BUILD_GAME_SO       ?= 0
BUILD_RENDERER_OPENGL ?=0
BUILD_RENDERER_JS   ?= 0
BUILD_RENDERER_OPENGL2 ?=0
BUILD_RENDERER_OPENGLES ?=0

USE_GAME_DLOPEN     ?= 0
USE_CURL_DLOPEN     ?= 0
USE_RENDERER_DLOPEN ?= 0
USE_BOTLIB_DLOPEN   ?= 0
SDL_LOADSO_DLOPEN   ?= 0
USE_OPENAL_DLOPEN   ?= 0
USE_RMLUI_DLOPEN    ?= 0
USE_OPUS_DLOPEN     ?= 0
USE_FREETYPE_DLOPEN ?= 0

USE_SDL             ?= 0
USE_CURL            ?= 0
USE_LOCAL_HEADERS   ?= 0
USE_VULKAN          ?= 0
USE_JPEG            ?= 0
USE_VULKAN_API      ?= 0
USE_Q3KEY           ?= 0
USE_IPV6            ?= 0
USE_CODEC_VORBIS    ?= 0
USE_CODEC_OPUS      ?= 0
USE_VIDEO_THEORA    ?= 0
USE_CIN_OGM         ?= 0
USE_CIN_WEBM        ?= 0

USE_FREETYPE        ?= 0
USE_MUMBLE          ?= 0
USE_VOIP            ?= 0
USE_LOCAL_HEADERS   ?= 0
GL_EXT_direct_state_access ?=1
GL_ARB_ES2_compatibility ?=1
GL_GLEXT_PROTOTYPES ?= 1

USE_SYSTEM_ZLIB     ?= 0
USE_SYSTEM_LIBC     ?= 1
USE_SYSTEM_CURL     ?= 0
USE_SYSTEM_JPEG     ?= 0
USE_SYSTEM_BOTLIB   ?= 0
USE_SYSTEM_OGG      ?= 0
USE_SYSTEM_OPUS     ?= 0
USE_SYSTEM_FREETPYE ?= 0

ifndef TEMPDIR
TEMPDIR=/tmp
endif

ifndef COPYDIR
COPYDIR="/usr/local/games/quake3"
endif

ifndef DESTDIR
DESTDIR=/usr/local/games/quake3
endif

ifndef GENERATE_DEPENDENCIES
GENERATE_DEPENDENCIES=1
endif

ifndef USE_CCACHE
USE_CCACHE=0
endif
export USE_CCACHE

ifndef USE_LOCAL_HEADERS
USE_LOCAL_HEADERS=1
endif

ifndef USE_CURL_DLOPEN
ifdef MINGW
  USE_CURL_DLOPEN=0
else
  USE_CURL_DLOPEN=1
endif
endif

ifneq ($(USE_RENDERER_DLOPEN),0)
#  USE_VULKAN=1
endif

ifneq ($(USE_VULKAN),0)
  USE_VULKAN_API=1
endif

#############################################################################

CC              := gcc
CXX             := c++
GXX             := g++
ASM             := yasm
LD              := $(CC)

bin_path=$(shell which $(1) 2> /dev/null)

STRIP ?= strip
PKG_CONFIG ?= pkg-config

# TODO: if USE_INTERNAL_* is requested, add this to prebuild steps
ifneq ($(call bin_path, $(PKG_CONFIG)),)
  SDL_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags-only-I sdl2)
  SDL_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs sdl2)
  X11_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags-only-I x11)
  X11_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs x11)
  FREETYPE_CFLAGS  ?= $(shell $(PKG_CONFIG) --silence-errors --cflags freetype2 || true)
  FREETYPE_LIBS    ?= $(shell $(PKG_CONFIG) --silence-errors --libs freetype2 || echo -lfreetype)
  THEORA_CFLAGS    ?= $(shell $(PKG_CONFIG) --silence-errors --cflags theora || true)
  THEORA_LIBS      ?= $(shell $(PKG_CONFIG) --silence-errors --libs theora || echo -ltheora)
  XVID_CFLAGS      ?= $(shell $(PKG_CONFIG) --silence-errors --cflags xvidcore || true)
  XVID_LIBS        ?= $(shell $(PKG_CONFIG) --silence-errors --libs xvidcore || echo -lxvidcore)
  OPUS_CFLAGS      ?= $(shell $(PKG_CONFIG) --silence-errors --cflags opusfile opus || true)
  OPUS_LIBS        ?= $(shell $(PKG_CONFIG) --silence-errors --libs opusfile opus || echo -lopusfile -lopus)
  VORBIS_CFLAGS    ?= $(shell $(PKG_CONFIG) --silence-errors --cflags vorbisfile vorbis || true)
  VORBIS_LIBS      ?= $(shell $(PKG_CONFIG) --silence-errors --libs vorbisfile vorbis || echo -lvorbisfile -lvorbis)
  OGG_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags ogg vorbis || true)
  OGG_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs ogg vorbis || echo -logg -lvorbis)
  OPENSSL_CFLAGS   ?= -I/usr/local/Cellar/openssl@1.1/1.1.1k/include
  OPENSSL_LIBS     ?= -L/usr/local/Cellar/openssl@1.1/1.1.1k/lib -lssl \
                      -L/usr/local/Cellar/openssl@1.1/1.1.1k/lib -lcrypto \
                      -lz
  SSH_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags ssh2 || true)
  SSH_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs ssh2 || echo -lssh2)
  GLIB_CFLAGS      ?= $(shell $(PKG_CONFIG) --silence-errors --cflags glib-2.0 || true)
  GLIB_LIBS        ?= $(shell $(PKG_CONFIG) --silence-errors --libs glib-2.0 || echo -lg)
  XML_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libxml-2.0 || true) \
                      -I/usr/include/libxml2
  XML_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs libxml-2.0 || echo -lxml2) \
                      -L/usr/local/opt/libxml2/lib
  STD_CFLAGS       ?= 
  STD_LIBS         ?= -lstdc++
  JPEG_CFLAGS      ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libjpeg || true)
  JPEG_LIBS        ?= $(shell $(PKG_CONFIG) --silence-errors --libs libjpeg || echo -ljpeg)
  PNG_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libpng || true)
  PNG_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs libpng || echo -lpng)
  VPX_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libvpx libwebm || true)
  VPX_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs libvpx libwebm || echo -lvpx -lwebm)
  CURL_CFLAGS      ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libcurl || true)
  CURL_LIBS        ?= $(shell $(PKG_CONFIG) --silence-errors --libs libcurl || echo -lcurl)
  ZLIB_CFLAGS      ?= $(shell $(PKG_CONFIG) --silence-errors --cflags zlib || true)
  ZLIB_LIBS        ?= $(shell $(PKG_CONFIG) --silence-errors --libs zlib || echo -lz)

endif

SYSROOT      := $(shell xcrun --show-sdk-path)
#SYSROOT       := /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk

# supply some reasonable defaults for SDL/X11?
ifeq ($(X11_CFLAGS),)
X11_CFLAGS = -I/usr/X11R6/include
endif
ifeq ($(X11_LIBS),)
X11_LIBS = -lX11
endif
ifeq ($(SDL_LIBS),)
SDL_LIBS = -lSDL2
endif

# extract version info
VERSION=$(shell grep "\#define Q3_VERSION" $(CMDIR)/q_shared.h | \
          sed -e 's/.*"[^" ]* \([^" ]*\)\( MV\)*"/\1/')

# common qvm definition
ifeq ($(ARCH),x86_64)
  HAVE_VM_COMPILED = true
else
ifeq ($(ARCH),x86)
  HAVE_VM_COMPILED = true
else
  HAVE_VM_COMPILED = false
endif
endif

ifeq ($(ARCH),arm)
  HAVE_VM_COMPILED = true
endif
ifeq ($(ARCH),aarch64)
  HAVE_VM_COMPILED = true
endif

BASE_CFLAGS   :=

ifeq ($(USE_PRINT_CONSOLE),1)
  BASE_CFLAGS += -DUSE_PRINT_CONSOLE
endif

ifeq ($(USE_PERSIST_CONSOLE),1)
  BASE_CFLAGS += -DUSE_PERSIST_CONSOLE
endif

ifeq ($(USE_NO_CONSOLE),1)
  BASE_CFLAGS += -DUSE_NO_CONSOLE
endif

ifeq ($(BUILD_GAME_STATIC),1)
  BASE_CFLAGS += -DBUILD_GAME_STATIC
endif

ifeq ($(USE_ASYNCHRONOUS),1)
  BASE_CFLAGS += -DUSE_ASYNCHRONOUS
endif

ifeq ($(USE_MEMORY_MAPS),1)
  BASE_CFLAGS += -DUSE_MEMORY_MAPS
endif

ifeq ($(USE_MULTIVM_CLIENT),1)
  BASE_CFLAGS += -DUSE_MULTIVM_CLIENT -DUSE_MV
endif

ifeq ($(USE_MULTIVM_SERVER),1)
  BASE_CFLAGS += -DUSE_MULTIVM_SERVER -DUSE_MV
endif

ifeq ($(USE_CVAR_UNCHEAT),1)
  BASE_CFLAGS += -DUSE_CVAR_UNCHEAT
endif

ifeq ($(USE_SYSTEM_JPEG),1)
  BASE_CFLAGS += -DUSE_SYSTEM_JPEG
endif

ifeq ($(USE_DYNAMIC_ZIP),1)
  BASE_CFLAGS += -DUSE_DYNAMIC_ZIP
endif

ifeq ($(USE_DEMO_CLIENTS),1)
  BASE_CFLAGS += -DUSE_DEMO_CLIENTS
endif

ifeq ($(USE_DEMO_SERVER),1)
  BASE_CFLAGS += -DUSE_DEMO_CLIENTS
endif

ifndef RENDERER_PREFIX
  RENDERER_PREFIX  := $(CNAME)
endif

ifndef BOTLIB_PREFIX
  BOTLIB_PREFIX    := $(CNAME)
endif

ifndef Q3MAP2_PREFIX
  Q3MAP2_PREFIX    := $(CNAME)
endif

ifndef MBSPC_PREFIX
  MBSPC_PREFIX     := $(CNAME)
endif

ifeq ($(USE_MULTIVM_CLIENT),1)
  BOTLIB_PREFIX    := $(CNAME)_mw
  RENDERER_PREFIX  := $(CNAME)_mw
endif
ifeq ($(USE_MULTIVM_SERVER),1)
  BOTLIB_PREFIX    := $(CNAME)_mw
  RENDERER_PREFIX  := $(CNAME)_mw
endif

ifeq ($(USE_BOTLIB_DLOPEN),1)
  BASE_CFLAGS += -DUSE_BOTLIB_DLOPEN
  BASE_CFLAGS += -DBOTLIB_PREFIX=\\\"$(BOTLIB_PREFIX)\\\"
endif

ifneq ($(USE_RENDERER_DLOPEN),0)
  BASE_CFLAGS += -DUSE_RENDERER_DLOPEN
  BASE_CFLAGS += -DRENDERER_PREFIX=\\\"$(RENDERER_PREFIX)\\\"
endif

ifeq ($(USE_CODEC_VORBIS),1)
  BASE_CFLAGS += -DUSE_CODEC_VORBIS=1
endif

ifeq ($(USE_BSP1),1)
  BASE_CFLAGS += -DUSE_BSP1
endif

ifeq ($(USE_CIN_WEBM),1)
  BASE_CFLAGS += -DUSE_CIN_WEBM=1
endif

ifeq ($(USE_CIN_OGM),1)
  BASE_CFLAGS += -DUSE_CIN_OGM=1
endif

ifeq ($(USE_VIDEO_THEORA),1)
  BASE_CFLAGS += -DUSE_VIDEO_THEORA=1
endif

ifeq ($(USE_VIDEO_XVID),1)
  BASE_CFLAGS += -DUSE_VIDEO_XVID=1
endif

ifeq ($(USE_CIN_VPX),1)
  BASE_CFLAGS += -DUSE_CIN_VPX=1
endif

ifeq ($(USE_DEDICATED),1)
  BASE_CFLAGS += -DDEDICATED
endif

ifeq ($(BUILD_SLIM_CLIENT),1)
  BASE_CFLAGS += -DBUILD_SLIM_CLIENT
endif

ifeq ($(USE_RMLUI),1)
  BASE_CFLAGS += -DUSE_RMLUI=1
endif

ifeq ($(USE_RMLUI_DLOPEN),1)
  BASE_CFLAGS += -DUSE_RMLUI_DLOPEN=1
endif

ifeq ($(USE_DRAGDROP),1)
  BASE_CFLAGS += -DUSE_DRAGDROP=1
endif

ifdef DEFAULT_BASEDIR
  BASE_CFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\"
endif

ifeq ($(USE_LOCAL_HEADERS),1)
  BASE_CFLAGS += -DUSE_LOCAL_HEADERS=1
endif

ifeq ($(USE_VULKAN_API),1)
  BASE_CFLAGS += -DUSE_VULKAN_API
endif

ifeq ($(GENERATE_DEPENDENCIES),1)
  BASE_CFLAGS += -MMD
endif

## Defaults
ARCHEXT=

CLIENT_EXTRA_FILES=
#GIT_VERSION := $(shell git describe --abbrev=6 --dirty --always --tags)

ifneq ($(HAVE_VM_COMPILED),true)
  BASE_CFLAGS += -DNO_VM_COMPILED
endif

ifeq ($(BUILD_STANDALONE),1)
  BASE_CFLAGS += -DSTANDALONE
endif

ifeq ($(USE_Q3KEY),1)
  BASE_CFLAGS += -DUSE_Q3KEY -DUSE_MD5
endif

ifeq ($(NOFPU),1)
  BASE_CFLAGS += -DNOFPU
endif

ifeq ($(USE_CURL),1)
  BASE_CFLAGS += -DUSE_CURL
ifeq ($(USE_CURL_DLOPEN),1)
  BASE_CFLAGS += -DUSE_CURL_DLOPEN
else
ifeq ($(MINGW),1)
  BASE_CFLAGS += -DCURL_STATICLIB
endif
endif
endif

help:
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.DEFAULT_GOAL := release
.RECIPEPREFIX +=
