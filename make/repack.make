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




################ PLATFORM SPECIFIC COMMANDS 


# TODO: make script for repack files, just like repack.js but no graphing
# TODO: use _hi _lo formats and the renderer and same directory to load converted web content
# TODO: figure out how this fits in with AMP file-naming style
_                 = $() $()
MKFILE           := $(lastword $(MAKEFILE_LIST))
ZIP              := zip
CONVERT          := convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 15% -auto-orient 
ENCODE           := oggenc --downmix --resample 11025 --quiet 
UNZIP            := unzip -n 
IDENTIFY         := identify -format '%[opaque]'
REPACK_MOD       := 1
NO_OVERWRITE     ?= 1
GETEXT            = $(if $(filter "%True%",$(shell $(IDENTIFY) "$(1)")),jpg,png)
FILTER_EXT        = $(foreach ext,$(1), $(filter %.$(ext),$(2)))
LVLS             := * */* */*/* */*/*/* */*/*/*/*
LEVELS            = $(subst $(1)/,,$(subst \space$(1)/, ,$(foreach lvl,$(LVLS), $(subst $(_),\space,$(wildcard $(1)/$(lvl))))))
LEVELS_PK3        = $(subst $(1)/,,$(subst \space$(1)/, ,$(foreach lvl,$(LVLS), $(subst $(_),\space,$(wildcard $(1)/$(lvl).pk3dir)))))
REPLACE_EXT       = $(foreach ext,$(1), $(subst .$(ext),.%,$(2)))
COPY             := cp
UNLINK           := rm
MOVE             := mv
MKDIR            ?= mkdir -p


RPK_TARGET       := $(PK3_PREFIX).pk3
RPK_CONVERT      := $(subst .pk3,-converted,$(RPK_TARGET))
RPK_ENCODE       := $(subst .pk3,-encoded,$(RPK_TARGET))
RPK_UNPACK       := $(notdir $(wildcard $(SRCDIR)/*.pk3))
RPK_PK3DIRS      := $(subst .pk3dir,.pk3,$(notdir $(wildcard $(SRCDIR)/*.pk3dir)))
RPK_TARGETS      := $(RPK_TARGET) $(RPK_UNPACK) $(RPK_PK3DIRS)
RPK_WORKDIRS     := $(addsuffix dir,$(RPK_TARGETS))
RPK_REPLACE       = $(subst \space, ,$(subst $(DESTDIR)/$(RPK_TARGET)dir/,$(SRCDIR)/,$(1)))
RPK_LOCAL         = $(subst \space, ,$(subst $(DESTDIR)/$(RPK_TARGET)-packed/,,$(1)))
RPK_GETEXT        = $(basename $(1)).$(call GETEXT,$(call RPK_REPLACE,$(1)))


################ BUILD OBJS / DIFF FILES 


# must convert files in 2 seperate steps because of images 
#   we don't know what format we end up with until after the
#   conversion so we don't repeat the the GETEXT command excessively
FILES_SRCPK3     := $(call LEVELS_PK3,$(SRCDIR))
FILES_SRC        := $(call LEVELS,$(SRCDIR))
ALL_FILES        := $(filter-out $(FILES_SRCPK3),$(FILES_SRC))
FILES_DONEPK3    := $(call LEVELS_PK3,$(DESTDIR)/$(RPK_TARGET)dir)
FILES_DONE       := $(call LEVELS,$(DESTDIR)/$(RPK_TARGET)dir)
ALL_DONE         := $(filter-out $(FILES_DONEPK3),$(FILES_DONE))

IMAGE_VALID_EXTS := jpg png
IMAGE_CONV_EXTS  := dds tga bmp pcx
IMAGE_ALL_EXTS   := $(IMAGE_CONV_EXTS) $(IMAGE_VALID_EXTS)
AUDIO_VALID_EXTS := ogg
AUDIO_CONV_EXTS  := wav mp3 opus
AUDIO_ALL_EXTS   := $(AUDIO_CONV_EXTS) $(AUDIO_VALID_EXTS)
FILE_ALL_EXT     := cfg skin menu shaderx mtr arena bot txt shader





################ DO WORK DEFINES

define DO_CONVERT
	$(echo_cmd) "CONVERT $(call RPK_REPLACE,$@) -> $@"
	$(Q)$(MKDIR) "$(subst \space, ,$(dir $@))"
	$(Q)$(CONVERT) "$(call RPK_REPLACE,$@)" "$(subst \space, ,$(call RPK_GETEXT,$@))"
endef

define DO_UNPACK
	$(echo_cmd) "UNPACK $<"
	$(Q)$(UNZIP) "$<" -d "$@dir/" > /dev/null
endef

define DO_ENCODE
	$(echo_cmd) "ENCODE $(call RPK_REPLACE,$@) -> $@"
	$(Q)$(MKDIR) "$(subst \space, ,$(dir $@))"
	$(Q)$(ENCODE) "$(call RPK_REPLACE,$@)" -n "$(subst \space, ,$(basename $@)).ogg"
endef

define DO_COPY
	$(echo_cmd) "COPY $(call RPK_REPLACE,$@) -> $@"
	$(Q)$(MKDIR) "$(dir $@)"
	$(Q)$(COPY) -n "$(call RPK_REPLACE,$@)" "$(subst \space, ,$@)"
endef

define DO_ARCHIVE
	$(echo_cmd) "ARCHIVE $@"
	$(Q)pushd "$(DESTDIR)/$(RPK_TARGET)dir" > /dev/null && \
	$(ZIP) -o ../$(PK3_PREFIX).zip "$(subst -packed,dir,$(call RPK_LOCAL,$@))" > /dev/null && \
	popd > /dev/null
endef



################### TARGETS / GOALS

ifdef TARGET_CONVERT

# list images with converted pathname then check for existing alt-name in 
#   defined script
# convert jpg from source dirs in case there is a quality conversion
IMAGE_SRC        := $(call FILTER_EXT,$(IMAGE_ALL_EXTS),$(ALL_FILES))
IMAGE_DONE       := $(call FILTER_EXT,$(IMAGE_VALID_EXTS),$(ALL_DONE))
IMAGE_DONE_WILD  := $(call REPLACE_EXT,$(IMAGE_VALID_EXTS),$(IMAGE_DONE))
IMAGE_NEEDED     := $(filter-out $(IMAGE_DONE_WILD),$(IMAGE_SRC))
IMAGE_OBJS       := $(addprefix $(DESTDIR)/$(RPK_TARGET)dir/,$(IMAGE_NEEDED))

convert: $(TARGET_CONVERT)
	@:

$(DESTDIR)/$(RPK_TARGET)dir/%:
	$(DO_CONVERT)

$(DESTDIR)/$(PK3_PREFIX)-converted: $(IMAGE_OBJS)
	$(echo_cmd) "CONVERTED $<"
	@:

endif # TARGET_CONVERT





ifdef TARGET_ENCODE

# list images with converted pathname then check for existing alt-name in 
#   defined script
# convert jpg from source dirs in case there is a quality conversion
AUDIO_SRC        := $(call FILTER_EXT,$(AUDIO_ALL_EXTS),$(ALL_FILES))
AUDIO_DONE       := $(call FILTER_EXT,$(AUDIO_VALID_EXTS),$(ALL_DONE))
AUDIO_DONE_WILD  := $(call REPLACE_EXT,$(AUDIO_VALID_EXTS),$(AUDIO_DONE))
AUDIO_NEEDED     := $(filter-out $(AUDIO_DONE_WILD),$(AUDIO_SRC))
AUDIO_OBJS       := $(addprefix $(DESTDIR)/$(RPK_TARGET)dir/,$(AUDIO_NEEDED))

encode: $(TARGET_ENCODE)
	@:

$(DESTDIR)/$(RPK_TARGET)dir/%:
	$(DO_ENCODE)

$(DESTDIR)/$(PK3_PREFIX)-encoded: $(AUDIO_OBJS)
	$(echo_cmd) "ENCODED $<"
	@:

endif # TARGET_ENCODE





ifdef TARGET_UNPACK

unpack: $(TARGET_UNPACK)
	$(echo_cmd) "UNPACKED $<"
	@:

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time
$(DESTDIR)/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK)

endif





ifdef TARGET_MKDIRS

mkdirs: $(TARGET_MKDIRS)
	$(echo_cmd) "MADEDIRS $<"
	@:

$(DESTDIR)/%.pk3dir: 
	@if [ ! -d "./$(DESTDIR)/" ];then $(MKDIR) "./$(DESTDIR)";fi
	@if [ ! -d "./$@" ];then $(MKDIR) "./$@";fi

endif







ifdef TARGET_REPACK

IMAGE_SRC        := $(call FILTER_EXT,$(IMAGE_ALL_EXTS),$(ALL_FILES))
IMAGE_SRCWILD    := $(call REPLACE_EXT,$(IMAGE_ALL_EXTS),$(IMAGE_SRC))
IMAGE_DESTINED   := $(addprefix $(DESTDIR)/$(RPK_TARGET)-packed/,$(filter $(IMAGE_SRCWILD),$(ALL_DONE)))

package: $(TARGET_REPACK)
	$(echo_cmd) "PACKAGED $<"
	-@$(MOVE) $(DESTDIR)/$(RPK_TARGET) $(DESTDIR)/$(RPK_TARGET).bak > /dev/null
	$(Q)$(MOVE) $(DESTDIR)/$(PK3_PREFIX).zip $(DESTDIR)/$(RPK_TARGET)
	-@$(UNLINK) $(DESTDIR)/$(RPK_TARGET).bak > /dev/null

$(DESTDIR)/$(RPK_TARGET)-packed/%:
	$(DO_ARCHIVE)

$(DESTDIR)/$(PK3_PREFIX)-packed: $(IMAGE_DESTINED)
	@:

endif







################### MAIN / REPACK!

repack:
	$(echo_cmd) "REPACK $(SRCDIR) $(RPK_TARGETS)"
	@$(MAKE) -f $(MKFILE) V=$(V) mkdirs -j 8 \
		TARGET_MKDIRS="$(addprefix $(DESTDIR)/,$(RPK_WORKDIRS))"
	@$(MAKE) -f $(MKFILE) V=$(V) unpack -j 8 \
		TARGET_UNPACK="$(addprefix $(DESTDIR)/,$(RPK_UNPACK))"
	@$(MAKE) -f $(MKFILE) V=$(V) convert -j 8 \
		TARGET_CONVERT="$(addprefix $(DESTDIR)/,$(RPK_CONVERT))"
	@$(MAKE) -f $(MKFILE) V=$(V) encode -j 8 \
		TARGET_ENCODE="$(addprefix $(DESTDIR)/,$(RPK_ENCODE))"
	@$(MAKE) -f $(MKFILE) V=$(V) package -j 1 \
		TARGET_REPACK="$(DESTDIR)/$(PK3_PREFIX)-packed"

