MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := libwebm

BUILD_LIBWEBM := 1
include make/platform.make

TARGET	      := libwebm_$(SHLIBNAME)
SOURCES       := libs/libwebm-1.0
INCLUDES      := $(SOURCES) libs/vpx-1.10
LIBS          := $(VPX_LIBS) 

WEBMOBJS      := mkvmuxer/mkvmuxer.o mkvmuxer/mkvmuxerutil.o mkvmuxer/mkvwriter.o
WEBMOBJS      += mkvparser/mkvparser.o mkvparser/mkvreader.o
WEBMOBJS      += common/file_util.o common/hdr_util.o
WEBMOBJS      += webmdec.o
Q3OBJ         := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(WEBMOBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS        := $(INCLUDE) -fsigned-char -MMD \
                 -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
CXXFLAGS      := $(CFLAGS) -std=gnu++11

define DO_WEBM_GXX
  $(echo_cmd) "WEBM_GXX $<"
  $(Q)$(GXX) -o $@ $(SHLIBCFLAGS) $(CXXFLAGS) -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) MAKEDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) MAKEDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.cc
	$(DO_WEBM_GXX)

$(B)/$(WORKDIR)/%.o: $(SOURCES)/common/%.cc
	$(DO_WEBM_GXX)
	
$(B)/$(WORKDIR)/%.o: $(SOURCES)/mkvmuxer/%.cc
	$(DO_WEBM_GXX)

$(B)/$(WORKDIR)/%.o: $(SOURCES)/mkvparser/%.cc
	$(DO_WEBM_GXX)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(GXX) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS) 
endif
