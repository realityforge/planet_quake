# most make files are the format 
# 1) PLATFORM
# 2) BUILD OBJS
# 3) DEFINES
# 4) GOALS
#this make file adds an additional BUILD OBJS and defined down below

# TODO: make script for repack files, just like repack.js but no graphing
# TODO: use _hi _lo formats and the renderer and same directory to load converted web content
# TODO: figure out how this fits in with AMP file-naming style
MKFILE           := $(lastword $(MAKEFILE_LIST))
ZIP              := zip
CONVERT          := convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 15% -auto-orient 
UNZIP            := unzip -n 
IDENTIFY         := identify -format '%[opaque]'
REPACK_MOD       := 1
NO_OVERWRITE     ?= 1
GETEXT            = $(if $(filter "%True%",$(shell $(IDENTIFY) "$(1)")),jpg,png)
#BOTH              = $(1).png $(1).jpg

include make/platform.make

ifndef SRCDIR
SRCDIR :=
endif
ifeq ($(SRCDIR),)
ifeq ($(PLATFORM),darwin)
SRCDIR := /Applications/ioquake3/baseq3
else

endif
endif

ifeq ($(SRCDIR),)
$(error No SRCDIR!)
endif

BASEMOD          := $(notdir $(SRCDIR))
WORKDIRS         := $(BASEMOD)-converted
PK3DIRS          := $(wildcard $(SRCDIR)/*.pk3)
PK3OBJS          := $(patsubst $(SRCDIR)/%,$(B)/$(BASEMOD)-converted/%,$(PK3DIRS))

define DO_CONVERT_CC
	$(echo_cmd) "CONVERT $(subst $(SRCDIR)/,,$<)"
	$(Q)$(MKDIR) "$(dir $@)"
	$(Q)$(CONVERT) "$<" "$(basename $@).$(call GETEXT,$<)"
endef

define DO_UNPACK_CC
	$(echo_cmd) "UNPACK $<"
	$(Q)$(UNZIP) "$<" -d "$@dir/"
endef


debug:
	$(echo_cmd) "REPACK $(SRCDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS="$(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(BD)/$(BASEMOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 16 \
		TARGET_MOD="$(BASEMOD).pk3" $(BD)/$(BASEMOD).collect
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) \
		TARGET_MOD="$(BASEMOD).pk3" $(BD)/$(BASEMOD).pk3

release:
	$(echo_cmd) "REPACK $(WORKDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(BR)/$(BASEMOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 16 \
		TARGET_MOD="$(BASEMOD).pk3" $(BR)/$(BASEMOD).collect
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) \
		TARGET_MOD="$(BASEMOD).pk3" $(BR)/$(BASEMOD).pk3

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time

$(B)/$(BASEMOD)-converted/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK_CC)
$(BR)/$(BASEMOD).unpacked: $(PK3OBJS)
	$(echo_cmd) "UNPACKED $@"
$(BD)/$(BASEMOD).unpacked: $(PK3OBJS)
	$(echo_cmd) "UNPACKED $@"


ifdef TARGET_MOD

# does it help to store file searches down here?
ALLFILES         := $(wildcard $(SRCDIR)/*) \
                    $(wildcard $(SRCDIR)/*/*) \
										$(wildcard $(SRCDIR)/*/*/*) \
										$(wildcard $(SRCDIR)/*/*/*/*) \
										$(wildcard $(SRCDIR)/*/*/*/*/*)
DONEFILES        := $(wildcard $(B)/$(BASEMOD)-converted/*) \
                    $(wildcard $(B)/$(BASEMOD)-converted/*/*) \
										$(wildcard $(B)/$(BASEMOD)-converted/*/*/*) \
										$(wildcard $(B)/$(BASEMOD)-converted/*/*/*/*) \
										$(wildcard $(B)/$(BASEMOD)-converted/*/*/*/*/*)

# skip checking image for transparency
DONE             := $(filter %.jpg, $(DONEFILES)) $(filter %.png, $(DONEFILES))
DONE_STRIPPED    := $(subst $(B)/$(BASEMOD)-converted/,,$(DONE))
DONE_TGAS        := $(patsubst %.png,%.tga,$(patsubst %.jpg,%.tga,$(DONE_STRIPPED)))
IMAGES           := $(filter %.tga, $(ALLFILES))
IMAGES_STRIPPED  := $(subst $(SRCDIR)/,,$(IMAGES))

CONVERTIMAGES    := $(filter-out $(DONE_TGAS),$(IMAGES_STRIPPED))
IMAGES_NEEDED    := $(addprefix $(B)/$(BASEMOD)-converted/,$(CONVERTIMAGES))
IMAGES_INCLUDED  := $(filter $(IMAGES_STRIPPED:.tga=.png),$(DONE_STRIPPED)) \
                    $(filter $(IMAGES_STRIPPED:.tga=.jpg),$(DONE_STRIPPED))
IMAGEOBJS        := $(addprefix $(B)/$(BASEMOD)-converted/,$(IMAGES_INCLUDED))

$(B)/$(BASEMOD)-converted/%.tga: $(SRCDIR)/%.tga
	@:
# $(DO_CONVERT_CC)

#BOTHLIST := $(foreach x,$(LIST),$(call BOTH,$(x)))

$(B)/$(BASEMOD).collect: $(IMAGES_NEEDED) $(IMAGEOBJS)
	$(echo_cmd) "DONE COLLECTING $@"

ifneq ($(IMAGES_NEEDED),)
$(B)/$(TARGET_MOD): $(IMAGEOBJS)
	@error something didn't work because image conversion is still needed
else
$(B)/$(TARGET_MOD): $(IMAGEOBJS)
	$(Q)$(ZIP) -o $@ $(IMAGEOBJS)
endif

endif