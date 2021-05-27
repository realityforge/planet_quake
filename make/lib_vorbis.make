MKFILE      := $(lastword $(MAKEFILE_LIST))
WORKDIR     := libogg
WORKDIR2    := libvorbis

include make/configure.make
include make/platform.make

TARGET	    := libogg_
TARGET2	    := libvorbis_
SOURCES     := libs/libogg-1.3.4/src
SOURCES2    := libs/libvorbis-1.3.7/lib
INCLUDES    := 
LIBS 				:=

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
	@$(MAKE) -f $(MKFILE) B=$(BD) BUILD_LIBOGG=1 $(BD)/$(TARGET)$(SHLIBNAME)
	@$(MAKE) -f $(MKFILE) B=$(BD) BUILD_LIBVORBIS=1 $(BD)/$(TARGET2)$(SHLIBNAME)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR2) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) BUILD_LIBOGG=1 $(BR)/$(TARGET)$(SHLIBNAME)
	@$(MAKE) -f $(MKFILE) B=$(BR) BUILD_LIBVORBIS=1 $(BR)/$(TARGET2)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.c
	$(DO_OGG_CC)

$(B)/$(WORKDIR2)/%.o: $(SOURCES2)/%.c
	$(DO_VORBIS_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)

$(B)/$(TARGET2)$(SHLIBNAME): $(Q3OBJ2) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
endif
