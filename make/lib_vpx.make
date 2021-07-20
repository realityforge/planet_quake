MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := libvpx

BUILD_LIBVPX := 1
include make/platform.make

TARGET	      := libvpx_$(SHLIBNAME)
SOURCES       := libs/libvpx-1.10
INCLUDES      := $(SOURCES) \
								 libs/libvorbis-1.3.7/include \
								 libs/opus-1.3.1/include \
								 libs/libogg-1.3.4/include \
								 libs/libvpx-1.10/third_party/libwebm
LIBS          := $(VPX_LIBS) $(VORBIS_LIBS) $(OPUS_LIBS)

CFILES        := webmdec.cc
VPXOBJS       := 
VPXOBJS       += 
OBJS          := $(CFILES:.cc=.o)
Q3OBJ         := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS        := $(INCLUDE) -fsigned-char -MMD \
                 -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
GXXFLAGS      := $(CFLAGS) -std=gnu++11

define DO_VPX_GXX
  $(echo_cmd) "VPX_GXX $<"
  $(Q)$(GXX) -o $@ $(SHLIBCFLAGS) $(GXXFLAGS) -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf ./$(BD)/$(WORKDIR) ./$(BD)/$(TARGET)
	@rm -rf ./$(BR)/$(WORKDIR) ./$(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.cc
	$(DO_VPX_GXX)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(GXX) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS) 
endif
