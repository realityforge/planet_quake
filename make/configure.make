
CNAME            = quake3e
DNAME            = quake3e.ded

BUILD_CLIENT     = 1
BUILD_SLIM_CLIENT= 1
BUILD_SERVER     = 1
BUILD_GAMES      = 0
BUILD_LIBS       = 0
BUILD_GAME_QVM   = 0
BUILD_GAME_SO    = 0
BUILD_RENDERER_OPENGL=0
BUILD_RENDERER_JS=0
BUILD_RENDERER_OPENGL2=0
BUILD_RENDERER_OPENGLES=0

USE_SDL          = 1
USE_CURL         = 1
USE_LOCAL_HEADERS= 0
USE_VULKAN       = 0
USE_SYSTEM_JPEG  = 0
USE_VULKAN_API   = 0
USE_Q3KEY        = 0
USE_IPV6         = 0
USE_SDL          = 1
USE_CURL         = 0
USE_CURL_DLOPEN  = 0
USE_CODEC_VORBIS = 0
USE_CODEC_OPUS   = 0
USE_FREETYPE     = 0
USE_MUMBLE       = 0
USE_VOIP         = 0
SDL_LOADSO_DLOPEN= 0
USE_OPENAL_DLOPEN= 0
USE_RENDERER_DLOPEN=1
USE_LOCAL_HEADERS=0
GL_EXT_direct_state_access=1
GL_ARB_ES2_compatibility=1
GL_GLEXT_PROTOTYPES=1

USE_SYSTEM_BOTLIB=1

ifndef COPYDIR
COPYDIR="/usr/local/games/quake3"
endif

ifndef DESTDIR
DESTDIR=/usr/local/games/quake3
endif

ifndef MOUNT_DIR
MOUNT_DIR=code
endif

ifndef BUILD_DIR
BUILD_DIR=build
endif

ifndef GENERATE_DEPENDENCIES
GENERATE_DEPENDENCIES=1
endif

ifndef USE_CCACHE
USE_CCACHE=0
endif
export USE_CCACHE

ifndef USE_CODEC_VORBIS
USE_CODEC_VORBIS=0
endif

ifndef USE_CODEC_OPUS
USE_CODEC_OPUS=0
endif

ifndef USE_CIN_THEORA
USE_CIN_THEORA=0
endif

ifndef USE_CIN_XVID
USE_CIN_THEORA=0
endif

ifndef USE_CIN_VPX
USE_CIN_THEORA=0
endif

ifndef USE_LOCAL_HEADERS
USE_LOCAL_HEADERS=1
endif

ifndef USE_CURL
USE_CURL=1
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

bin_path=$(shell which $(1) 2> /dev/null)

STRIP ?= strip
PKG_CONFIG ?= pkg-config

ifneq ($(call bin_path, $(PKG_CONFIG)),)
  SDL_INCLUDE ?= $(shell $(PKG_CONFIG) --silence-errors --cflags-only-I sdl2)
  SDL_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs sdl2)
  X11_INCLUDE ?= $(shell $(PKG_CONFIG) --silence-errors --cflags-only-I x11)
  X11_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs x11)
  FREETYPE_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags freetype2 || true)
  FREETYPE_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs freetype2 || echo -lfreetype)
endif

# supply some reasonable defaults for SDL/X11?
ifeq ($(X11_INCLUDE),)
X11_INCLUDE = -I/usr/X11R6/include
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

BASE_CFLAGS =

ifndef USE_MEMORY_MAPS
  USE_MEMORY_MAPS = 1
  BASE_CFLAGS += -DUSE_MEMORY_MAPS
endif

ifeq ($(USE_SYSTEM_JPEG),1)
  BASE_CFLAGS += -DUSE_SYSTEM_JPEG
endif

ifneq ($(HAVE_VM_COMPILED),true)
  BASE_CFLAGS += -DNO_VM_COMPILED
endif

ifndef RENDERER_PREFIX
  RENDERER_PREFIX  := $(CNAME)
endif

ifneq ($(USE_RENDERER_DLOPEN),0)
  BASE_CFLAGS += -DUSE_RENDERER_DLOPEN
  BASE_CFLAGS += -DRENDERER_PREFIX=\\\"$(RENDERER_PREFIX)\\\"
endif

ifeq ($(USE_CODEC_VORBIS),1)
  BASE_CFLAGS += -DUSE_CODEC_VORBIS=1
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
INSTALL=install
MKDIR=mkdir

ARCHEXT=

CLIENT_EXTRA_FILES=
GIT_VERSION := $(shell git describe --abbrev=6 --dirty --always --tags)

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

-include Makefile.local

#############################################################################
# DEPENDENCIES
#############################################################################

help:
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

D_FILES :=

ifdef WORKDIR

mkdirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/$(WORKDIR) ];then $(MKDIR) $(B)/$(WORKDIR);fi

D_FILES := $(@shell find $(BD)/$(WORKDIR) -name '*.d') \
					 $(@shell find $(BR)/$(WORKDIR) -name '*.d')
ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean makedirs release \
	targets tools toolsclean mkdirs \
		$(D_FILES)

.DEFAULT_GOAL := default
