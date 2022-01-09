
bin_path=$(shell which $(1) 2> /dev/null)

STRIP ?= strip
PKG_CONFIG ?= pkg-config

# TODO: if USE_INTERNAL_* is requested, add this to prebuild steps
ifneq ($(call bin_path, $(PKG_CONFIG)),)
  SDL_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags-only-I sdl2 || true)
  SDL_LIBS         ?= $(shell $(PKG_CONFIG) --silence-errors --libs sdl2 || echo -lSDL2)
  X11_CFLAGS       ?= $(shell $(PKG_CONFIG) --silence-errors --cflags-only-I x11 || true)
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

# extract version info
VERSION=$(shell grep "\#define Q3_VERSION" $(CMDIR)/q_shared.h | \
          sed -e 's/.*"[^" ]* \([^" ]*\)\( MV\)*"/\1/')
