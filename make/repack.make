# TODO: make script for repack files, just like repack.js but no graphing
# TODO: use _hi _lo formats and the renderer and same directory to load converted web content
# TODO: figure out how this fits in with AMP file-naming style
MKFILE           := $(lastword $(MAKEFILE_LIST))
ZIP              := zip
CONVERT          := convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 15% -auto-orient 
IDENTIFY         := identify -format '%[opaque]'
REPACK_MOD       := 1
NO_OVERWRITE     ?= 1

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

WORKDIR          := $(notdir $(SRCDIR))

ifeq ($(WORKDIR),)
$(error No SRCDIR!)
endif

WORKDIRS         := $(WORKDIR)-c $(WORKDIR)-cc
TARGET_MOD       := $(WORKDIR).pk3
IMAGES           := $(wildcard $(SRCDIR)/*.tga) \
                    $(wildcard $(SRCDIR)/*/*.tga) \
										$(wildcard $(SRCDIR)/*/*/*.tga) \
										$(wildcard $(SRCDIR)/*/*/*/*.tga) \
										$(wildcard $(SRCDIR)/*/*/*/*/*.tga)

#TODO: skip checking image for transparency
#both = $(1).ml $(1).mli
#BOTHLIST := $(foreach x,$(LIST),$(call both,$(x)))


debug:
	$(echo_cmd) "REPACK $(WORKDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS="$(WORKDIR) $(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 8 \
		WORKDIRS="$(WORKDIR) $(WORKDIRS)" \
		CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" \
		$(BD)/$(TARGET_MOD)

release:
	$(echo_cmd) "REPACK $(WORKDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(WORKDIR) $(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 8 \
		WORKDIRS="$(WORKDIR) $(WORKDIRS)" \
		CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" \
		$(BR)/$(TARGET_MOD)

ifdef B
FILES            := $(IMAGES)
MODOBJS          := $(patsubst $(SRCDIR)/%,$(B)/$(WORKDIR)-cc/%,$(FILES))
GETEXT            = $(if $(filter "%False%",$(shell $(IDENTIFY) "$(1)")),png,jpg)

$(B)/$(WORKDIR)-cc/%.tga: $(SRCDIR)/%.tga
	$(echo_cmd) "CONVERT $<"
	$(Q)$(MKDIR) "$(dir $@)"
	$(Q)$(CONVERT) "$<" "$(basename $@).$(call GETEXT,$<)"

$(B)/$(WORKDIR)-cc: $(MODOBJS)

$(B)/$(TARGET_MOD): $(B)/$(WORKDIR)-cc
	$(echo_cmd) "ZIP $@"
	$(Q)$(ZIP) -o $@ $(MODOBJS) 
endif