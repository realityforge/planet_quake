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
TARGET_MOD       := $(notdir $(SRCDIR)).pk3

define DO_CONVERT_CC
	$(echo_cmd) "CONVERT $<"
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
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(BD)/$(TARGET_MOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 16 $(BD)/$(TARGET_MOD)

release:
	$(echo_cmd) "REPACK $(WORKDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(BR)/$(TARGET_MOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 16 $(BR)/$(TARGET_MOD)

ifdef B
PK3DIRS          := $(wildcard $(SRCDIR)/*.pk3)
PK3OBJS          := $(patsubst $(SRCDIR)/%,$(B)/$(BASEMOD)-converted/%,$(PK3DIRS))

ALLFILES         := $(wildcard $(SRCDIR)/*) \
                    $(wildcard $(SRCDIR)/*/*) \
										$(wildcard $(SRCDIR)/*/*/*) \
										$(wildcard $(SRCDIR)/*/*/*/*) \
										$(wildcard $(SRCDIR)/*/*/*/*/*)
IMAGES           := $(filter-out /%.tga, $(ALLFILES))
FILES            := $(IMAGES)
MODOBJS          := $(patsubst $(SRCDIR)/%,$(B)/$(BASEMOD)-converted/%,$(FILES))

$(B)/$(BASEMOD)-converted/%.tga: $(SRCDIR)/%.tga
	$(DO_CONVERT_CC)

$(B)/$(BASEMOD)-converted/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK_CC)

#TODO: skip checking image for transparency
#BOTHLIST := $(foreach x,$(LIST),$(call BOTH,$(x)))

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time
$(B)/$(TARGET_MOD).unpacked: $(PK3OBJS)
	$(echo_cmd) "UNPACKED $@"

$(B)/$(TARGET_MOD): $(MODOBJS)
	$(echo_cmd) "ZIP $@"
	$(Q)$(ZIP) -o $@ -u $(MODOBJS)
endif