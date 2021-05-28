MKFILE       := $(lastword $(MAKEFILE_LIST))
WORKDIR      := libogg
WORKDIR2     := libvorbis

BUILD_VORBIS := 1
BUILD_OGG    := 1
include make/platform.make

TARGET	     := libogg_$(SHLIBNAME)
TARGET2	     := libvorbis_$(SHLIBNAME)
SOURCES      := libs/libogg-1.3.4/src
SOURCES2     := libs/libvorbis-1.3.7/lib
INCLUDES     := 
LIBS 				 :=

OGGOBJS     := bitwise.o \
               framing.o

VORBISOBJS  := analysis.o \
							 bitrate.o \
							 block.o \
							 codebook.o \
							 envelope.o \
							 floor0.o \
							 floor1.o \
							 info.o \
							 lookup.o \
							 lpc.o \
							 lsp.o \
							 mapping0.o \
							 mdct.o \
							 psy.o \
							 registry.o \
							 res0.o \
							 smallft.o \
							 sharedbook.o \
							 synthesis.o \
							 vorbisfile.o \
							 window.o

Q3OBJ       := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OGGOBJS)))
Q3OBJ2       := $(addprefix $(B)/$(WORKDIR2)/,$(notdir $(VORBISOBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS      := $(INCLUDE) -fsigned-char -MMD \
               -O2 -ftree-vectorize -ffast-math -fno-short-enums

define DO_OGG_CC
  @echo "OGG_CC $<"
  @$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

define DO_VORBIS_CC
  @echo "VORBIS_CC $<"
  @$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=$(WORKDIR2) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) BUILD_LIBOGG=1 $(BD)/$(TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BD) BUILD_LIBVORBIS=1 $(BD)/$(TARGET2)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR2) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) BUILD_LIBOGG=1 $(BR)/$(TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BR) BUILD_LIBVORBIS=1 $(BR)/$(TARGET2)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.c
	$(DO_OGG_CC)

$(B)/$(WORKDIR2)/%.o: $(SOURCES2)/%.c
	$(DO_VORBIS_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)

$(B)/$(TARGET2): $(Q3OBJ2) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
