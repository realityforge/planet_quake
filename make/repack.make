#.DEFAULT_GOAL := repack
# most make files are the format 
# 1) PLATFORM
# 2) BUILD OBJS
# 3) DEFINES
# 4) GOALS
#this make file adds an additional BUILD OBJS and defined down below
ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

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
FILTER_EXT        = $(foreach ext,$(1), $(filter %.$(ext),$(2)))
COPY             := cp
UNLINK           := rm
MOVE             := mv
MKDIR            ?= mkdir -p

ifndef SRCDIR
SRCDIR           := games/multigame/assets
PK3_PREFIX       := xxx-multigame
endif

ifndef PK3_PREFIX
PK3_PREFIX       := $(subst .pk3dir,,$(notdir $(SRCDIR)))
endif

ifeq ($(PK3_PREFIX),multigame)
PK3_PREFIX       := xxx-multigame
endif

ifeq ($(SRCDIR),)
$(error No SRCDIR!)
endif

ifndef DESTDIR
DESTDIR          := build
endif

ifeq ($(DESTDIR),)
$(error No SRCDIR!)
endif

ifndef RPK_EXT
RPK_EXT          := pk3
endif

ifeq ($(NO_REPACK),1)
RPK_EXT          := pk3dir
endif

RPK_LEVELS       := * */* */*/* */*/*/* */*/*/*/*
RPK_TARGET       := $(PK3_PREFIX).pk3
RPK_UNPACK       := $(notdir $(wildcard $(SRCDIR)/*.pk3))
RPK_PK3DIRS      := $(subst .pk3dir,.pk3,$(notdir $(wildcard $(SRCDIR)/*.pk3dir)))
RPK_TARGETS      := $(RPK_TARGET) $(RPK_UNPACK) $(RPK_PK3DIRS)
RPK_WORKDIRS     := $(addsuffix dir,$(RPK_TARGETS))

# must convert files in 2 seperate steps because of images 
#   we don't know what format we end up with until after the
#   conversion so we don't repeat the the GETEXT command excessively
RPK_CONVERTED    :=

PK3DIR_FILES     := $(foreach lvl,$(RPK_LEVELS), $(wildcard $(SRCDIR)/$(lvl).pk3dir))
SOURCE_FILES     := $(foreach lvl,$(RPK_LEVELS), $(wildcard $(SRCDIR)/$(lvl)))
ALL_FILES        := $(subst $(SRCDIR)/,,$(filter-out $(PK3DIR_FILES),$(SOURCE_FILES)))

# list images with converted pathname then check for existing alt-name in 
#   defined script
IMAGE_VALID      := jpg png
IMAGE_CONVERT    := dds tga bmp pcx
IMAGE_NEEDED     := $(call FILTER_EXT,$(IMAGE_CONVERT),$(ALL_FILES))
IMAGE_OBJS       := $(addprefix $(DESTDIR)/$(RPK_TARGET)dir/,$(IMAGE_NEEDED))

ifneq ($(IMAGE_NEEDED),)
RPK_CONVERTED    += $(subst .pk3,-converted.pk3,$(RPK_TARGET))
endif

#RPK_CONVERTED    := $(subst .pk3,-converted.pk3,$(RPK_TARGETS))


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

$(DESTDIR)/%.pk3dir: 
	@if [ ! -d $(DESTDIR) ];then $(MKDIR) $(DESTDIR);fi
	@if [ ! -d "./$(DESTDIR)/$@" ];then $(MKDIR) "./$(DESTDIR)/$@";fi

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time
$(DESTDIR)/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK_CC)

$(DESTDIR)/$(RPK_TARGET)dir/%: $(SRCDIR)/%
	$(DO_CONVERT_CC)


$(DESTDIR)/$(PK3_PREFIX)-converted.pk3: $(SRCDIR) $(IMAGE_OBJS)
	$(echo_cmd) "CONVERT $(IMAGE_NEEDED)"
	@:

#$(DESTDIR)/%-converted.pk3: $(SRCDIR)/%.pk3
#	$(echo_cmd) "CONVERT PK3 $@"
#	@:

#$(DESTDIR)/%-converted.pk3: $(SRCDIR)/%.pk3dir
#	$(echo_cmd) "CONVERT PK3DIR $@"
#	@:

repack:
	$(echo_cmd) "REPACK $(RPK_TARGETS)"
	@$(MAKE) -f $(MKFILE) V=$(V) $(addprefix $(DESTDIR)/,$(RPK_WORKDIRS))
	@$(MAKE) -f $(MKFILE) V=$(V) $(addprefix $(DESTDIR)/,$(RPK_UNPACK))
	@$(MAKE) -f $(MKFILE) V=$(V) $(addprefix $(DESTDIR)/,$(RPK_CONVERTED))

#	@$(MAKE) -f $(MKFILE) V=$(V) -j 16 \
#		TARGET_MOD="$(PK3_PREFIX).zip" $(BD)/$(BASEMOD).collect
#	@$(MAKE) -f $(MKFILE) V=$(V) -j 1 \
#		TARGET_MOD="$(PK3_PREFIX).zip" $(BD)/$(PK3_PREFIX).zip
#	@$(MAKE) -f $(MKFILE) V=$(V) -j 1 \
#		TARGET_MOD="$(PK3_PREFIX).zip" $(BD)/$(BASEMOD).pk3dirs


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
VALID_EXT        := $(addprefix %.,$(VALID_IMG_EXT))
CONVERT_EXT      := $(addprefix %.,$(CONVERT_IMG_EXT))
include make/difflist.make
IMG_NEEDED       := $(addprefix $(B)/$(BASEMOD)-images/,$(DIFFLIST_CONVERT))
IMG_OBJS         := $(filter-out  ,$(addprefix $(B)/$(BASEMOD)-images/,$(DIFFLIST_INCLUDED)))


VALID_SND_EXT    := ogg
CONVERT_SND_EXT  := wav mp3
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


