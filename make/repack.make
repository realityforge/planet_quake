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
SRCDIR           := games/multigame/assets
PK3_PREFIX       := xxx-multigame
endif

ifeq ($(SRCDIR),)
$(error No SRCDIR!)
endif

BASEMOD          := $(subst .pk3dir,,$(notdir $(SRCDIR)))
ifeq ($(BASEMOD),multigame)
PK3_PREFIX       := xxx-multigame
endif

ifndef PK3_PREFIX
PK3_PREFIX       := $(BASEMOD)
endif
WORKDIRS         := $(BASEMOD)-sounds $(BASEMOD)-images $(BASEMOD)-unpacked
PK3_FILES        := $(wildcard $(SRCDIR)/*.pk3)
PK3OBJS          := $(patsubst $(SRCDIR)/%,$(B)/$(BASEMOD)-unpacked/%,$(PK3DIRS))

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
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(BD)/$(BASEMOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 16 \
		TARGET_MOD="$(PK3_PREFIX).zip" $(BD)/$(BASEMOD).collect
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 1 \
		TARGET_MOD="$(PK3_PREFIX).zip" $(BD)/$(PK3_PREFIX).zip
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 1 \
		TARGET_MOD="$(PK3_PREFIX).zip" $(BD)/$(BASEMOD).pk3dirs

release:
	$(echo_cmd) "REPACK $(WORKDIR)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(BR)/$(BASEMOD).unpacked
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 16 \
		TARGET_MOD="$(PK3_PREFIX).zip" $(BR)/$(BASEMOD).collect
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 1 \
		TARGET_MOD="$(PK3_PREFIX).zip" $(BR)/$(PK3_PREFIX).zip
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) -j 1 \
		TARGET_MOD="$(PK3_PREFIX).zip" $(BR)/$(BASEMOD).pk3dirs

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time



ifdef B

$(B)/$(BASEMOD)-unpacked/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK_CC)

$(B)/$(BASEMOD).unpacked: $(PK3OBJS)
	$(echo_cmd) "UNPACKED $@"

#BOTHLIST := $(foreach x,$(LIST),$(call BOTH,$(x)))

endif



ifdef TARGET_MOD

# does it help to store file searches down here?
PK3DIR_FILES     := $(wildcard $(SRCDIR)/*.pk3dir) \
                    $(wildcard $(SRCDIR)/*.pk3dir/*) \
                    $(wildcard $(SRCDIR)/*.pk3dir/*/*) \
                    $(wildcard $(SRCDIR)/*.pk3dir/*/*/*) \
                    $(wildcard $(SRCDIR)/*.pk3dir/*/*/*/*)
SOURCE_FILES     := $(wildcard $(SRCDIR)/*) \
                    $(wildcard $(SRCDIR)/*/*) \
                    $(wildcard $(SRCDIR)/*/*/*) \
                    $(wildcard $(SRCDIR)/*/*/*/*) \
                    $(wildcard $(SRCDIR)/*/*/*/*/*)
ALL_FILES        := $(filter-out $(PK3DIR_FILES),$(SOURCE_FILES))
SOURCE_STRIPPED  := $(subst $(SRCDIR)/,,$(ALL_FILES))
DONEPK3_FILES    := $(wildcard $(B)/$(BASEMOD)-*.pk3dir/*) \
                    $(wildcard $(B)/$(BASEMOD)-*.pk3dir/*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*.pk3dir/*/*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*.pk3dir/*/*/*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*.pk3dir/*/*/*/*/*)
DONE_FILES       := $(wildcard $(B)/$(BASEMOD)-*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*/*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*/*/*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*/*/*/*/*) \
                    $(wildcard $(B)/$(BASEMOD)-*/*/*/*/*/*)
DONENOPK3_FILES  := $(filter-out $(DONEPK3_FILES), $(DONE_FILES))
DONE_STRIPPED    := $(subst $(B)/$(BASEMOD)-images/,,$(DONENOPK3_FILES)) \
                    $(subst $(B)/$(BASEMOD)-sounds/,,$(DONENOPK3_FILES))


# skip checking image for transparency
VALID_IMG_EXT    := jpg png
CONVERT_IMG_EXT  := dds tga bmp pcx
VALID_EXT        := $(addprefix %.,$(VALID_IMG_EXT))
CONVERT_EXT      := $(addprefix %.,$(CONVERT_IMG_EXT))
include make/difflist.make
IMG_NEEDED       := $(addprefix $(B)/$(BASEMOD)-images/,$(DIFFLIST_CONVERT))
IMG_OBJS         := $(filter-out  ,$(addprefix $(B)/$(BASEMOD)-images/,$(DIFFLIST_INCLUDED)))


VALID_SND_EXT    := mp3 ogg
CONVERT_SND_EXT  := wav 
VALID_EXT        := $(addprefix %.,$(VALID_SND_EXT))
CONVERT_EXT      := $(addprefix %.,$(CONVERT_SND_EXT))
include make/difflist.make
SND_NEEDED       := $(addprefix $(B)/$(BASEMOD)-sounds/,$(DIFFLIST_CONVERT))
SND_OBJS         := $(filter-out  ,$(addprefix $(B)/$(BASEMOD)-sounds/,$(DIFFLIST_INCLUDED)))


VALID_FILE_EXT   := cfg skin menu shaderx mtr arena bot txt shader $(INCLUDE_EXTS)
FILE_INCLUDED    := $(filter-out  ,$(foreach vext,$(addprefix %.,$(VALID_FILE_EXT)), $(filter $(vext), $(ALL_FILES))))
PK3_DIRECTORIES  := $(addprefix $(B)/pk3dirs/,$(subst $(SRCDIR)/,,$(wildcard $(SRCDIR)/*.pk3dir)))
VMS_INCLUDED    := $(filter-out  ,$(filter %.qvm, $(ALL_FILES)))


ifneq ($(IMG_NEEDED),)
CONVERSION_NEEDED := 1
endif
ifneq ($(SND_NEEDED),)
CONVERSION_NEEDED := 1
endif

ifdef CONVERSION_NEEDED

$(B)/$(BASEMOD)-images/%: $(SRCDIR)/%
	$(DO_CONVERT_CC)

$(B)/$(BASEMOD)-sounds/%: $(SRCDIR)/%
	$(DO_ENCODE_CC)

$(B)/$(BASEMOD).collect: $(IMG_NEEDED) $(IMG_OBJS) $(SND_NEEDED) $(SND_OBJS) $(FILE_INCLUDED) $(VMS_INCLUDED)
	$(echo_cmd) "DONE COLLECTING $@"

$(B)/$(PK3_PREFIX).zip: $(IMG_OBJS) $(SND_OBJS) $(FILE_INCLUDED) $(VMS_INCLUDED)
	@echo $(IMG_NEEDED)
	@echo "something went wrong because there are still files to convert"
	exit 1



else



$(B)/$(BASEMOD).collect: $(IMG_NEEDED) $(IMG_OBJS) $(SND_NEEDED) $(SND_OBJS) $(FILE_INCLUDED) $(VMS_INCLUDED)
	$(echo_cmd) "DONE COLLECTING $@"

ifneq ($(VMS_INCLUDED),)
$(B)/$(PK3_PREFIX)-vms.zip: $(VMS_INCLUDED)
	$(echo_cmd) "ZIPPING $<"
	$(Q)pushd $(SRCDIR) && \
		$(ZIP) -o $(PK3_PREFIX)-vms.zip $(subst $(SRCDIR)/,,$(VMS_INCLUDED)) && \
		popd
	-@$(MOVE) $(B)/$(PK3_PREFIX)-vms.pk3 $(B)/$(PK3_PREFIX)-vms.pk3.bak
	$(Q)$(MOVE) $(SRCDIR)/$(PK3_PREFIX)-vms.zip $(B)/$(PK3_PREFIX)-vms.pk3
	-@$(UNLINK) $(B)/$(PK3_PREFIX)-vms.pk3.bak
else
$(B)/$(PK3_PREFIX)-vms.zip: $(IMG_OBJS)
	$(echo_cmd) "SKIPPING, no VMs $<"
endif

ifneq ($(FILE_INCLUDED),)
$(B)/$(PK3_PREFIX)-files.zip: $(FILE_INCLUDED)
	$(echo_cmd) "ZIPPING $<"
	$(Q)pushd $(SRCDIR) && \
		$(ZIP) -o $(PK3_PREFIX)-files.zip $(subst $(SRCDIR)/,,$(FILE_INCLUDED)) && \
		popd
	-@$(MOVE) $(B)/$(PK3_PREFIX)-files.pk3 $(B)/$(PK3_PREFIX)-files.pk3.bak
	$(Q)$(MOVE) $(SRCDIR)/$(PK3_PREFIX)-files.zip $(B)/$(PK3_PREFIX)-files.pk3
	-@$(UNLINK) $(B)/$(PK3_PREFIX)-files.pk3.bak
else
$(B)/$(PK3_PREFIX)-files.zip: $(IMG_OBJS)
	$(echo_cmd) "SKIPPING, no files $<"
endif

ifneq ($(IMG_OBJS),)
$(B)/$(PK3_PREFIX)-images.zip: $(IMG_OBJS)
	$(echo_cmd) "ZIPPING $<"
	$(Q)pushd $(B)/$(BASEMOD)-images && \
		$(ZIP) -o $(PK3_PREFIX)-images.zip $(subst $(B)/$(BASEMOD)-images/,,$(IMG_OBJS)) && \
		popd
	-@$(MOVE) $(B)/$(PK3_PREFIX)-images.pk3 $(B)/$(PK3_PREFIX)-images.pk3.bak
	$(Q)$(MOVE) $(B)/$(BASEMOD)-images/$(PK3_PREFIX)-images.zip $(B)/$(PK3_PREFIX)-images.pk3
	-@$(UNLINK) $(B)/$(PK3_PREFIX)-images.pk3.bak
else
$(B)/$(PK3_PREFIX)-images.zip: $(IMG_OBJS)
	$(echo_cmd) "SKIPPING, no images $<"
endif

ifneq ($(SND_OBJS),)
$(B)/$(PK3_PREFIX)-sounds.zip: $(SND_OBJS)
	$(echo_cmd) "ZIPPING $<"
	$(Q)pushd $(B)/$(BASEMOD)-sounds && \
		$(ZIP) -o $(PK3_PREFIX)-sounds.zip $(subst $(B)/$(BASEMOD)-sounds/,,$(SND_OBJS)) && \
		popd
	-@$(MOVE) $(B)/$(PK3_PREFIX)-sounds.pk3 $(B)/$(PK3_PREFIX)-sounds.pk3.bak
	$(Q)$(MOVE) $(B)/$(BASEMOD)-sounds/$(PK3_PREFIX)-sounds.zip $(B)/$(PK3_PREFIX)-sounds.pk3
	-@$(UNLINK) $(B)/$(PK3_PREFIX)-sounds.pk3.bak
else
$(B)/$(PK3_PREFIX)-sounds.zip: $(SND_OBJS)
	$(echo_cmd) "SKIPPING, no sounds $<"
endif

ALL_ZIPS := $(B)/$(PK3_PREFIX)-sounds.zip $(B)/$(PK3_PREFIX)-images.zip \
            $(B)/$(PK3_PREFIX)-files.zip $(B)/$(PK3_PREFIX)-vms.zip

$(B)/$(PK3_PREFIX).zip: $(ALL_ZIPS)
	$(echo_cmd) "DONE PACKING $<"

$(B)/pk3dirs/%.pk3dir: $(SRCDIR)/%.pk3dir
	$(echo_cmd) "MAKING $<"
	$(Q)$(MAKE) -f $(MKFILE) B=$(B) V=$(V) -j 1 \
		SRCDIR="$(SRCDIR)/${subst $(B)/pk3dirs/,,$@}" \
		TARGET_MOD="${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.zip" \
		INCLUDE_EXTS="bsp" \
		$(B)/${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.collect
	$(Q)$(MAKE) -f $(MKFILE) B=$(B) V=$(V) -j 1 \
		SRCDIR="$(SRCDIR)/${subst $(B)/pk3dirs/,,$@}" \
		TARGET_MOD="${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.zip" \
		INCLUDE_EXTS="bsp" \
		$(B)/${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.zip

$(B)/pk3dirs/%.pk3dir: $(B)/$(BASEMOD)-unpacked/%.pk3dir
	$(echo_cmd) "MAKING $<"
	$(Q)$(MAKE) -f $(MKFILE) B=$(B) V=$(V) -j 1 \
		SRCDIR="$(B)/$(BASEMOD)-unpacked/${subst $(B)/pk3dirs/,,$@}" \
		TARGET_MOD="${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.zip" \
		INCLUDE_EXTS="bsp" \
		$(B)/${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.collect
	$(Q)$(MAKE) -f $(MKFILE) B=$(B) V=$(V) -j 1 \
		SRCDIR="$(B)/$(BASEMOD)-unpacked/${subst $(B)/pk3dirs/,,$@}" \
		TARGET_MOD="${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.zip" \
		INCLUDE_EXTS="bsp" \
		$(B)/${subst .pk3dir,,${subst $(B)/pk3dirs/,,$@}}.zip

$(B)/$(BASEMOD).pk3dirs: $(PK3_DIRECTORIES)
	$(echo_cmd) "DONE PACKING PK3S $(PK3_DIRECTORIES)"



endif
endif


