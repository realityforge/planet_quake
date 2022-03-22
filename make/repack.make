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
ENCODE           := oggenc --downmix --resample 22050 --quiet 
UNZIP            := unzip -n 
IDENTIFY         := identify -format '%[opaque]'
REPACK_MOD       := 1
NO_OVERWRITE     ?= 1
GETEXT            = $(if $(filter "%True%",$(shell $(IDENTIFY) "$(1)")),jpg,png)
#BOTH              = $(1).png $(1).jpg
COPY             := cp
UNLINK           := rm
MOVE             := mv

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

define DO_ENCODE_CC
	$(echo_cmd) "ENCODE $(subst $(SRCDIR)/,,$<)"
	$(Q)$(MKDIR) "$(dir $@)"
	$(Q)$(ENCODE) "$<" -n "$(basename $@).ogg"
endef

define DO_COPY_CC
	$(echo_cmd) "COPY $(subst $(SRCDIR)/,,$<)"
	$(Q)$(MKDIR) "$(dir $@)"
	$(Q)$(COPY) -n "$<" "$@"
endef


debug:
	$(echo_cmd) "REPACK $(SRCDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS="$(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
#	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(BD)/$(BASEMOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 16 \
		TARGET_MOD="$(BASEMOD).zip" $(BD)/$(BASEMOD).collect
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) \
		TARGET_MOD="$(BASEMOD).zip" $(BD)/$(BASEMOD).zip

release:
	$(echo_cmd) "REPACK $(WORKDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
#	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(BR)/$(BASEMOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 16 \
		TARGET_MOD="$(BASEMOD).zip" $(BR)/$(BASEMOD).collect
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) \
		TARGET_MOD="$(BASEMOD).zip" $(BR)/$(BASEMOD).zip

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time



ifdef B

$(B)/$(BASEMOD)-converted/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK_CC)

$(B)/$(BASEMOD).unpacked: $(PK3OBJS)
	$(echo_cmd) "UNPACKED $@"

#BOTHLIST := $(foreach x,$(LIST),$(call BOTH,$(x)))

endif



ifdef TARGET_MOD

# does it help to store file searches down here?
ALLFILES         := $(wildcard $(SRCDIR)/*) \
                    $(wildcard $(SRCDIR)/*/*) \
										$(wildcard $(SRCDIR)/*/*/*) \
										$(wildcard $(SRCDIR)/*/*/*/*) \
										$(wildcard $(SRCDIR)/*/*/*/*/*)
ALL_STRIPPED     := $(subst $(SRCDIR)/,,$(ALLFILES))
DONEFILES        := $(wildcard $(B)/$(BASEMOD)-converted/*) \
                    $(wildcard $(B)/$(BASEMOD)-converted/*/*) \
										$(wildcard $(B)/$(BASEMOD)-converted/*/*/*) \
										$(wildcard $(B)/$(BASEMOD)-converted/*/*/*/*) \
										$(wildcard $(B)/$(BASEMOD)-converted/*/*/*/*/*)
DONE_STRIPPED    := $(subst $(B)/$(BASEMOD)-converted/,,$(DONEFILES))


# skip checking image for transparency
VALID_IMG_EXT    := %.jpg %.png
CONVERT_IMG_EXT  := %.dds %.tga %.bmp %.pcx
VALID_EXT        := $(VALID_IMG_EXT)
CONVERT_EXT      := $(CONVERT_IMG_EXT)
include make/difflist.make
IMG_NEEDED       := $(DIFFLIST_NEEDED)
IMG_OBJS         := $(DIFFLIST_OBJS)


VALID_SND_EXT    := %.mp3 %.ogg
CONVERT_SND_EXT  := %.wav 
VALID_EXT        := $(VALID_SND_EXT)
CONVERT_EXT      := $(CONVERT_SND_EXT)
include make/difflist.make
SND_NEEDED       := $(DIFFLIST_NEEDED)
SND_OBJS         := $(DIFFLIST_OBJS)

VALID_FILE_EXT   := %.cfg %.skin %.menu %.shader %.shaderx %.mtr %.arena %.bot $.txt
FILE_INCLUDED    := $(foreach vext,$(VALID_EXT), $(filter $(vext), $(DONE_STRIPPED)))
FILE_OBJS        := $(addprefix $(B)/$(BASEMOD)-converted/,$(FILE_INCLUDED))



ifneq ($(IMG_NEEDED),)
CONVERSION_NEEDED := 1
endif
ifneq ($(SND_NEEDED),)
CONVERSION_NEEDED := 1
endif

ifdef CONVERSION_NEEDED

$(B)/$(BASEMOD)-converted/%: $(SRCDIR)/%
	$(DO_COPY_CC)

$(B)/$(BASEMOD)-converted/%.tga: $(SRCDIR)/%.tga
	$(DO_CONVERT_CC)

$(B)/$(BASEMOD)-converted/%.wav: $(SRCDIR)/%.wav
	$(DO_ENCODE_CC)

$(B)/$(BASEMOD).collect: $(IMG_NEEDED) $(IMG_OBJS) $(SND_NEEDED) $(SND_OBJS) $(FILE_OBJS)
	$(echo_cmd) "DONE COLLECTING $@"

$(B)/$(BASEMOD).zip: $(IMG_OBJS) $(SND_OBJS) $(FILE_OBJS)
	@echo 
	@echo "something went wrong because there are still files to convert"
	exit 1

else

$(B)/$(BASEMOD).collect: $(IMG_NEEDED) $(IMG_OBJS) $(SND_NEEDED) $(SND_OBJS) $(FILE_OBJS)
	$(echo_cmd) "DONE COLLECTING $@"

$(B)/$(BASEMOD).zip: $(IMG_OBJS) $(SND_OBJS) $(FILE_OBJS)
	$(echo_cmd) "ZIPPING $<"
	$(Q)$(ZIP) -o $@ $(IMG_OBJS) $(SND_OBJS)
	$(Q)$(MOVE) $(B)/$(BASEMOD).pk3 $(B)/$(BASEMOD).pk3.bak
	$(Q)$(MOVE) $(B)/$(BASEMOD).zip $(B)/$(BASEMOD).pk3
	$(Q)$(UNLINK) $(B)/$(BASEMOD).pk3.bak
endif

endif






#DONE_SOUNDS      := $(filter %.mp3, $(DONEFILES)) $(filter %.ogg, $(DONEFILES))
#DONE_STRIPPED2   := $(subst $(B)/$(BASEMOD)-converted/,,$(DONE_SOUNDS))
#DONE_WAVS        := $(patsubst %.ogg,%.wav,$(patsubst %.mp3,%.wav,$(DONE_STRIPPED2)))
#WAVS             := $(filter %.wav, $(ALLFILES))
#WAVS_STRIPPED    := $(subst $(SRCDIR)/,,$(WAVS))
#CONVERT_WAVS     := $(filter-out $(DONE_WAVS),$(WAVS_STRIPPED))
#WAVS_NEEDED      := $(addprefix $(B)/$(BASEMOD)-converted/,$(CONVERT_WAVS))
#WAVS_INCLUDED    := $(filter $(WAVS_STRIPPED:.wav=.mp3),$(DONE_STRIPPED2)) \
#                    $(filter $(WAVS_STRIPPED:.wav=.ogg),$(DONE_STRIPPED2))
#WAVOBJS          := $(addprefix $(B)/$(BASEMOD)-converted/,$(WAVS_INCLUDED))
