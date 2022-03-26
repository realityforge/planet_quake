WORKOGG        := libogg
WORKVORBIS     := libvorbis
BUILD_VORBIS   := 1
BUILD_OGG      := 1

ifndef BUILD_CLIENT
MKFILE        := $(lastword $(MAKEFILE_LIST))
include make/platform.make
endif

TARGET_OGG     := libogg_$(SHLIBNAME)
TARGET_VORBIS  := libvorbis_$(SHLIBNAME)
SOURCES_OGG    := libs/libogg-1.3.4/src
SOURCES_VORBIS := libs/libvorbis-1.3.7/lib
INCLUDES       := libs/libogg-1.3.4/include libs/libvorbis-1.3.7/include

OGGOBJS        := bitwise.o framing.o
VORBISOBJS     := analysis.o bitrate.o block.o \
                  codebook.o envelope.o floor0.o \
                  floor1.o info.o lookup.o \
                  lpc.o lsp.o mapping0.o \
                  mdct.o psy.o registry.o \
                  res0.o smallft.o sharedbook.o \
                  synthesis.o vorbisfile.o window.o

OGG_OBJ        := $(addprefix $(B)/$(WORKOGG)/,$(notdir $(OGGOBJS)))
VORBIS_OBJ     := $(addprefix $(B)/$(WORKVORBIS)/,$(notdir $(VORBISOBJS)))

export OGG_INCLUDE := $(foreach dir,$(INCLUDES),-I$(dir))

LIBVOR_CFLAGS  := $(OGG_INCLUDE) $(BASE_CFLAGS) $(SHLIBCFLAGS) $(OGG_CFLAGS)
LIBVOR_LDFLAGS := $(OGG_INCLUDE) $(SHLIBLDFLAGS) $(OGG_LIBS)
LIBOGG_CFLAGS  := $(OGG_INCLUDE) $(BASE_CFLAGS) $(SHLIBCFLAGS)
LIBOGG_LDFLAGS := $(OGG_INCLUDE) $(SHLIBLDFLAGS)

define DO_OGG_CC
	$(echo_cmd) "OGG_CC $<"
	$(Q)$(CC) -o $@ $(LIBOGG_CFLAGS) -c $<
endef

define DO_VORBIS_CC
	$(echo_cmd) "VORBIS_CC $<"
	$(Q)$(CC) -o $@ $(LIBVOR_CFLAGS) -c $<
endef

ifndef BUILD_CLIENT

debug:
	$(echo_cmd) "MAKE $(TARGET_OGG)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIRS="$(WORKOGG) $(WORKVORBIS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) -j 8 \
		LIBVOR_CFLAGS="$(LIBVOR_CFLAGS) $(DEBUG_CFLAGS)" \
		LIBVOR_LDFLAGS="$(LIBVOR_LDFLAGS) $(DEBUG_LDFLAGS) $(BD)/libogg_$(SHLIBNAME)" \
		LIBOGG_CFLAGS="$(LIBOGG_CFLAGS) $(DEBUG_CFLAGS)" \
		LIBOGG_LDFLAGS="$(LIBOGG_LDFLAGS) $(DEBUG_LDFLAGS)" \
		$(BD)/$(TARGET_OGG) $(BD)/$(TARGET_VORBIS)

release:
	$(echo_cmd) "MAKE $(TARGET_OGG)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS="$(WORKOGG) $(WORKVORBIS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) -j 8 \
		LIBVOR_CFLAGS="$(LIBVOR_CFLAGS) $(RELEASE_CFLAGS)" \
		LIBVOR_LDFLAGS="$(LIBVOR_LDFLAGS) $(RELEASE_LDFLAGS) $(BR)/libogg_$(SHLIBNAME)" \
		LIBOGG_CFLAGS="$(LIBOGG_CFLAGS) $(RELEASE_CFLAGS)" \
		LIBOGG_LDFLAGS="$(LIBOGG_LDFLAGS) $(RELEASE_LDFLAGS)" \
		$(BR)/$(TARGET_OGG) $(BR)/$(TARGET_VORBIS)

clean:
	@rm -rf ./$(BD)/$(WORKOGG) ./$(BD)/$(TARGET_OGG)
	@rm -rf ./$(BR)/$(WORKOGG) ./$(BR)/$(TARGET_OGG)
	@rm -rf ./$(BD)/$(WORKVORBIS) ./$(BD)/$(TARGET_VORBIS)
	@rm -rf ./$(BR)/$(WORKVORBIS) ./$(BR)/$(TARGET_VORBIS)

else
WORKDIRS += $(WORKOGG) $(WORKVORBIS)
CLEANS += $(WORKOGG) $(WORKVORBIS) $(TARGET_OGG) $(TARGET_VORBIS)
endif

ifdef B
$(B)/$(WORKOGG)/%.o: $(SOURCES_OGG)/%.c
	$(DO_OGG_CC)

$(B)/$(WORKVORBIS)/%.o: $(SOURCES_VORBIS)/%.c
	$(DO_VORBIS_CC)

$(B)/$(TARGET_OGG): $(OGG_OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(OGG_OBJ) $(LIBOGG_CFLAGS) $(LIBOGG_LDFLAGS)

$(B)/$(TARGET_VORBIS): $(VORBIS_OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(VORBIS_OBJ) $(LIBVOR_CFLAGS) $(LIBVOR_LDFLAGS)
endif
