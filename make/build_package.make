# This script feels terrible. I don't like the defined functions and file lookups.
# Too much code for everything that it does. But I feel this way about most Makefiles. 
# Scaffolding should look like scaffolding?

# SRCDIR - games/multigame/assets/xxx-multigame.pk3dir
# SRCDIR | *.bsp,*.map | make $< -o $@

# The pipes look like studs. Or limp on with this system and build a parser 
#   and convert it all to bash/M$ subsystem?


# most .make files are the format 
# 1) PLATFORM
# 2) BUILD OBJECTS
# 3) DEFINES / FLAGS
# 4) GOALS
#this make file adds an additional BUILD OBJS and defines down below

# .do-always is used to force a command to run, even if the target files already exist
#   this is for things like putting all the images and js files inside quake3e.html
# some of these targets are idempotent, which means you can call it multiple times
#   and get the same/expected result every time, with minimal repetative work done

ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif
_                 = $() $()

ifndef SRCDIR
SRCDIR           := games/multigame/assets/xxx-multigame.pk3dir
endif

ifndef PK3_PREFIX
PK3_PREFIX       := $(subst $(_),\space,$(subst .pk3dir,,$(notdir $(SRCDIR))))
endif

ifeq ($(SRCDIR),games/multigame/assets/xxx-multigame.pk3dir)
# there is 2 reasons I put xxx- here. 1) because people associate it with the porn
# industry and it freaks them out to see it and jump to conclusions. 2) the real reason,
# so the packed pk3 filename is listed later to override the VMs in the earlier pk3s.
PK3_PREFIX       := xxx-multigame
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


############################################## PLATFORM SPECIFIC COMMANDS 

# TODO: make sure these are correct on every platform
# TODO: make script for repack files, just like repack.js but no graphing
# TODO: use _hi _lo formats and the renderer and same directory to load converted web content
# TODO: figure out how this fits in with AMP file-naming style
CNAME            := quake3e
NODE             := node
UGLIFY           := uglifyjs
OPT              := wasm-opt
MKFILE           := $(lastword $(MAKEFILE_LIST))
ZIP              := zip
CONVERT          := convert -strip -interlace Plane -sampling-factor 4:2:0 -quality 15% -auto-orient 
ENCODE           := oggenc -q 7 --downmix --resample 11025 --quiet 
FFMPEG           := ffmpeg 
UNZIP            := unzip -n 
IDENTIFY         := identify -format '%[opaque]'
REPACK_MOD       := 1
NO_OVERWRITE     ?= 1
GETEXT            = $(if $(filter "%True%",$(shell $(IDENTIFY) "$1")),jpg,png)
GETMPGA           = $(filter "%True%",$(shell $(NODE) -e "if(fs.readFileSync('$1', 'binary').includes('\x2E\x3D')) {console.log('True')}"))
FILTER_EXT        = $(foreach ext,$1, $(filter %.$(ext),$2))
LVLS4            := * */* */*/* */*/*/* */*/*/*/*
LVLS3            := * */* */*/* */*/*/*
LEVELS            = $(subst \pathsep,,$(subst \space\pathsep, ,$(foreach lvl,$(LVLS4), $(subst $(_),\space,$(subst $1/,\pathsep,$(wildcard $(subst $(_),\ ,$1)/$(lvl)))))))
LEVELS_PK3        = $(subst \pathsep,,$(subst \space\pathsep, ,$(foreach lvl,$(LVLS3), $(subst $(_),\space,$(subst $1/,\pathsep,$(wildcard $(subst $(_),\ ,$1)/*.pk3dir/$(lvl)))))))
REPLACE_EXT       = $(foreach ext,$1, $(subst .$(ext),.%,$2))
COPY             := cp
UNLINK           := rm
MOVE             := mv
MKDIR            ?= mkdir -p


RPK_TARGET       := $(PK3_PREFIX).pk3
RPK_DOALWAYS     := $(subst .pk3,.do-always,$(RPK_TARGET))
#RPK_UNPACK      := $(subst \pathsep,,$(subst \space\pathsep, ,$(subst $(_),\space,$(subst $(SRCDIR)/,\pathsep,$(wildcard $(SRCDIR)/*.pk3)))))
RPK_PK3DIRS      := $(subst .pk3dir,.pk3,$(subst \pathsep,,$(subst \space\pathsep, ,$(subst $(_),\space,$(subst $(SRCDIR)/,\pathsep,$(wildcard $(SRCDIR)/*.pk3dir))))))
RPK_TARGETS      := $(RPK_TARGET) $(RPK_UNPACK) $(RPK_PK3DIRS)
RPK_WORKDIRS     := $(addsuffix dir,$(RPK_TARGETS))
RPK_REPLACE       = $(subst \space, ,$(subst $(DESTDIR)/$(RPK_TARGET)dir/,$(SRCDIR)/,$1))
RPK_LOCAL         = $(subst \space, ,$(subst $(DESTDIR)/$(RPK_TARGET).do-always/,,$1))
RPK_COPY          = $(subst \space, ,$(subst $(DESTDIR)/$(PK3_PREFIX).do-always/,$(SRCDIR)/,$1))
RPK_COPIED        = $(subst \space, ,$(subst $(DESTDIR)/$(PK3_PREFIX).do-always/,$(DESTDIR)/$(RPK_TARGET)dir/,$1))
RPK_GETEXT        = $(basename $1).$(call GETEXT,$(call RPK_REPLACE,$1))
RPK_PK3DIR        = $(subst $(DESTDIR)/,$(SRCDIR)/,$(subst .do-always,.pk3dir,$1))
RPK_INDEXES      += $(notdir $(wildcard build/*/$(CNAME)*.wasm))


WASM_HTML        := index.html
WASM_HTTP        := code/wasm/http
WASM_FILES       := $(CNAME).js sys_emgl.js sys_fs.js sys_in.js \
                    sys_net.js sys_std.js sys_wasm.js nipplejs.js
WASM_JS          := $(addprefix $(WASM_HTTP)/,$(notdir $(WASM_FILES)))
WASM_ASSETS      := games/multigame/assets/xxx-multigame.pk3dir


NODE_FSWRITE      = fs.writeFileSync('$2', $1)
NODE_FSREAD       = fs.readFileSync('$1', 'utf-8')
NODE_FSREAD64     = fs.readFileSync('$1', 'base64')
NODE_FSREPLACE    = $(call NODE_FSWRITE,$(call NODE_FSREAD,$3).replace($1, $2),$3)
WASM_BASE64       = data:image/png;base64,'+$(call NODE_FSREAD64,$1)+'
HTML_VFSHEAD     := <!-- BEGIN VFS INCLUDES -->
HTML_SCRIPT       = <script async type=\"text/javascript\" src=\"$1\"></script>
HTML_LINK         = <link rel=\"stylesheet\" href=\"$1\" />
HTML_STYLE        = <style type=\"text/css\">\n/* <!-- $1 */\n'+$(call NODE_FSREAD,$2)+'\n/* --> */\n</style>
HTML_BODY         = </body>
JS_PREFS          = window.preFS[\\'$2\\']=\\''+$(call NODE_FSREAD64,$1)+'\\'\n
JS_SCRIPT         = <script async type=\"text/javascript\">\n/* <\!-- $2 */\n$1/* --> */\n</script>
#HTML_IMG          = 


ifeq ($(filter-out $(_),$(RPK_TARGETS)),)
$(error directory missing)
endif


################################################ BUILD OBJS / DIFF FILES 


############## DANGER ZONE
# because lvlworld conversion has so many files we limit automatic pk3 
#   packaging to .pk3dir names

ifneq ($(filter %.pk3dir,$(SRCDIR)),)

# must convert files in 2 seperate steps because of images 
#   we don't know what format we end up with until after the
#   conversion so we don't repeat the the GETEXT command excessively
FILES_SRCPK3     := $(call LEVELS_PK3,$(subst \space, ,$(SRCDIR)))
FILES_SRC        := $(call LEVELS,$(subst \space, ,$(SRCDIR)))
ALL_FILES        := $(filter-out $(FILES_SRCPK3),$(FILES_SRC))
FILES_DONEPK3    := $(call LEVELS_PK3,$(subst \space, ,$(DESTDIR)/$(RPK_TARGET)dir))
FILES_DONE       := $(call LEVELS,$(subst \space, ,$(DESTDIR)/$(RPK_TARGET)dir))
ALL_DONE         := $(filter-out $(FILES_DONEPK3),$(FILES_DONE))

ifeq ($(filter-out $(_),$(ALL_FILES)),)
$(error no files in source directory $(SRCDIR))
endif

endif

IMAGE_VALID_EXTS := jpg png
IMAGE_CONV_EXTS  := dds tga bmp pcx
IMAGE_ALL_EXTS   := $(IMAGE_CONV_EXTS) $(IMAGE_VALID_EXTS)
AUDIO_VALID_EXTS := ogg
AUDIO_CONV_EXTS  := wav mp3 opus mpga
AUDIO_ALL_EXTS   := $(AUDIO_CONV_EXTS) $(AUDIO_VALID_EXTS)
# TODO: remove lightmaps/vis from BSPs to save space
FILE_ALL_EXTS    := cfg skin menu shaderx mtr arena bot txt shader map bsp


################################################# DO WORK DEFINES


define DO_CONVERT
	$(echo_cmd) "CONVERT $(call RPK_REPLACE,$@) -> $@"
	$(Q)$(MKDIR) "$(subst \space, ,$(dir $@))"
	$(Q)$(CONVERT) "$(call RPK_REPLACE,$@)" "$(subst \space, ,$(call RPK_GETEXT,$@))"
endef

define DO_UNPACK
	$(echo_cmd) "UNPACK $<"
	$(Q)$(UNZIP) "$<" -d "$@dir/" 2> /dev/null
endef

define DO_ENCODE
	$(echo_cmd) "ENCODE $(call RPK_REPLACE,$@) -> $@"
	$(Q)$(MKDIR) "$(subst \space, ,$(dir $@))"
	$(Q)$(ENCODE) "$(call RPK_REPLACE,$@)" -n "$(subst \space, ,$(basename $@)).ogg"
endef

define DO_FFMPEG
	$(echo_cmd) "FFMPEG $(call RPK_REPLACE,$@) -> $@"
	$(Q)$(MKDIR) "$(subst \space, ,$(dir $@))"
	$(Q)$(FFMPEG) -i "$(call RPK_REPLACE,$@)" -c:a libvorbis -q:a 4 "$(subst \space, ,$(basename $@)).ogg"
endef

define DO_COPY
	$(echo_cmd) "COPY $(call RPK_COPY,$@) -> $@"
	$(Q)$(MKDIR) "$(call RPK_COPIED,$(dir $@))"
	$(Q)$(COPY) -n "$(call RPK_COPY,$@)" "$(call RPK_COPIED,$@)" ||:
endef

define DO_ARCHIVE
	$(echo_cmd) "ARCHIVE $1"
	$(Q)if [ -f "$(DESTDIR)/$3" ]; \
		then $(MOVE) "$(DESTDIR)/$3" "$2../$3";fi
	$(Q)pushd "$2" && \
	$(ZIP) -o ../$3 "$1" && \
	popd 2> /dev/null
	$(Q)$(MOVE) "$2../$3" "$(DESTDIR)/$3"
endef


########################################################### wasm indexing


define DO_OPT_CC
	$(echo_cmd) "OPT_CC $<"
	$(Q)$(OPT) -Os --no-validation -o $@ $<
	-$(Q)$(MOVE) $< $<.bak 2> /dev/null
	$(Q)$(MOVE) $@ $< 2> /dev/null
	-$(Q)$(UNLINK) $<.bak 2> /dev/null
endef

define DO_UGLY_CC
	$(echo_cmd) "UGLY_CC $<"
	$(Q)$(UGLIFY) $(WASM_JS) -o $@ -c -m
	-$(Q)$(MOVE) $(WASM_HTTP)/$(WASM_MIN) $(WASM_HTTP)/$(WASM_MIN).bak
  $(Q)$(MOVE) $@ $(WASM_HTTP)/$(WASM_MIN)
	-$(Q)$(UNLINK) $(WASM_HTTP)/$(WASM_MIN).bak
endef

define DO_BASE64_CC
	$(echo_cmd) "BASE64_CC $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_VFSHEAD)','$(HTML_VFSHEAD)\n<img title=\"$(subst $(SRCDIR)/,,$<)\" src=\"$(call WASM_BASE64,$<)\" />',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_CSS_EMBED
	$(echo_cmd) "CSS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(call HTML_LINK,$(subst $(WASM_HTTP)/,,$<))','$(call HTML_STYLE,$(notdir $<),$<)',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_JS_LIST
	$(echo_cmd) "JS_LIST $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(call HTML_SCRIPT,$(CNAME).js)','$(WASM_FILES)'.split(' ').map(jsFile => '$(call JS_SCRIPT,'+$(call NODE_FSREAD,$(WASM_HTTP)/'+jsFile+')+','+jsFile+')').join(''),$(DESTDIR)/$(WASM_HTML))"
endef

define DO_JSBUILD_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'<script async type=\"text/javascript\" src=\"${subst $(DESTDIR)/,,$<}\"></script>','$(call JS_SCRIPT,'+$(call NODE_FSREAD,$<)+',$(subst $(DESTDIR)/,,$<))',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_JS_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'<script async type=\"text/javascript\" src=\"${subst $(WASM_HTTP)/,,$<}\"></script>','$(call JS_SCRIPT,'+$(call NODE_FSREAD,$<)+',$(subst $(WASM_HTTP)/,,$<))',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_WASM_EMBED
	$(echo_cmd) "WASM_EMBED $@"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n$(call JS_SCRIPT,$(call JS_PREFS,$(subst .do-always,,$(@:.html=.wasm)),$(subst $(DESTDIR).do-always/,,$@)),$(subst $(DESTDIR)/,,$<))',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_ASSET_EMBED
	$(echo_cmd) "ASSET_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n<script async type=\"text/javascript\">\n/* <\!-- $(subst $(DESTDIR)/,,$<) */\nwindow.preFS[\\'multigame/$(notdir $<)\\']=\\''+$(call NODE_FSREAD64,$<)+'\\'\n/* --> */\n</script>',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_INDEX_CC
	$(echo_cmd) "INDEX_CC $@"
	$(Q)$(COPY) $(WASM_HTTP)/$(WASM_HTML) $(subst .do-always,,$@)
	$(Q)$(NODE) -e "console.assert('$(subst .do-always,,$@)' == '$(DESTDIR)/$(WASM_HTML)');$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n<script async type=\"text/javascript\">\nwindow.preStart=[\\'$(STARTUP_COMMAND)\\'];\n/* --> */\n</script>',$(DESTDIR)/$(WASM_HTML))"
endef


######################################################### TARGETS / GOALS


#################################################### image conversion


ifdef TARGET_CONVERT

# list images with converted pathname then check for existing alt-name in defined script
# convert jpg from source dirs in case there is a quality conversion
IMAGE_SRC        := $(call FILTER_EXT,$(IMAGE_ALL_EXTS),$(ALL_FILES))
IMAGE_DONE       := $(call FILTER_EXT,$(IMAGE_VALID_EXTS),$(ALL_DONE))
IMAGE_DONE_WILD  := $(call REPLACE_EXT,$(IMAGE_VALID_EXTS),$(IMAGE_DONE))
IMAGE_NEEDED     := $(filter-out $(IMAGE_DONE_WILD),$(IMAGE_SRC))
IMAGE_OBJS       := $(addprefix $(DESTDIR)/$(RPK_TARGET)dir/,$(IMAGE_NEEDED))


ifeq ($(IMAGE_OBJS),)

convert:
	$(echo_cmd) "NOTHING TO CONVERT"

else

convert: $(addprefix $(DESTDIR)/,$(TARGET_CONVERT))
	@:

endif

$(DESTDIR)/$(RPK_TARGET)dir/%.tga:
	$(NODE) -e "let wrong=fs.readFileSync('$(call RPK_REPLACE,$@)', 'binary');if(wrong.includes('created using ImageLib by SkyLine Tools')){wrong=wrong.replace('created using ImageLib by SkyLine Tools', '');wrong=wrong.replace(/^\(/, '\0');fs.writeFileSync('$(call RPK_REPLACE,$@)', wrong)}"
	$(DO_CONVERT)

$(DESTDIR)/$(RPK_TARGET)dir/%:
	$(DO_CONVERT)

$(DESTDIR)/$(PK3_PREFIX).do-always: $(IMAGE_OBJS)
	$(echo_cmd) "CONVERTED $<"

endif # TARGET_CONVERT


#################################################### audio conversion


ifdef TARGET_ENCODE

# list images with converted pathname then check for existing alt-name in 
#   defined script
# convert jpg from source dirs in case there is a quality conversion
AUDIO_SRC        := $(call FILTER_EXT,$(AUDIO_CONV_EXTS),$(ALL_FILES))
AUDIO_SRCDONE    := $(addprefix $(DESTDIR)/$(PK3_PREFIX).do-always/,$(call FILTER_EXT,$(AUDIO_VALID_EXTS),$(ALL_FILES)))
AUDIO_DONE       := $(call FILTER_EXT,$(AUDIO_VALID_EXTS),$(ALL_DONE))
AUDIO_DONE_WILD  := $(call REPLACE_EXT,$(AUDIO_VALID_EXTS),$(AUDIO_DONE))
AUDIO_NEEDED     := $(filter-out $(AUDIO_DONE_WILD),$(AUDIO_SRC))
AUDIO_OBJS       := $(addprefix $(DESTDIR)/$(RPK_TARGET)dir/,$(AUDIO_NEEDED))

ifeq ($(AUDIO_OBJS),)

encode:
	$(echo_cmd) "NOTHING TO ENCODE"

else

encode: $(addprefix $(DESTDIR)/,$(TARGET_ENCODE))
	@:

endif

$(DESTDIR)/$(RPK_TARGET)dir/%.mp3:
	$(DO_FFMPEG)

$(DESTDIR)/$(RPK_TARGET)dir/%.mpga:
	$(DO_FFMPEG)

$(DESTDIR)/$(RPK_TARGET)dir/%.wav:
	$(if $(call GETMPGA,$(call RPK_REPLACE,$@)),$(DO_ENCODE),$(DO_FFMPEG))

$(DESTDIR)/$(RPK_TARGET)dir/%:
	$(DO_ENCODE)

$(DESTDIR)/$(PK3_PREFIX).do-always/%:
	$(DO_COPY)

$(DESTDIR)/$(PK3_PREFIX).do-always: $(AUDIO_OBJS) $(AUDIO_SRCDONE)
	$(echo_cmd) "ENCODED $<"

endif # TARGET_ENCODE


################################################# pk3s extraction


ifdef TARGET_UNPACK

unpack: $(TARGET_UNPACK)
	$(echo_cmd) "UNPACKED $<"

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time
$(DESTDIR)/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK)

endif


###################################################### pk3dirs


ifdef TARGET_MKDIRS

mkdirs: $(addprefix $(DESTDIR)/,$(TARGET_MKDIRS))
	$(echo_cmd) "MADEDIRS $<"

$(DESTDIR)/%.pk3dir: 
	@if [ ! -d "$(subst \space, ,$(DESTDIR))/" ];then $(MKDIR) "$(subst \space, ,$(DESTDIR))";fi
	@if [ ! -d "$(subst \space, ,$@)" ];then $(MKDIR) "$(subst \space, ,$@)";fi

endif


#################################################### repack pk3 files


ifdef TARGET_REPACK

QVMS             := cgame.qvm qagame.qvm ui.qvm
QVM_SRC          := $(addprefix build/*/*/vm/,$(QVMS))
QVM_DESTINED     := $(addprefix $(DESTDIR)/$(RPK_TARGET).do-always-vms/,$(wildcard $(QVM_SRC)))

IMAGES_SRC       := $(call FILTER_EXT,$(IMAGE_ALL_EXTS),$(ALL_FILES))
IMAGES_SRCWILD   := $(call REPLACE_EXT,$(IMAGE_ALL_EXTS),$(IMAGES_SRC))
IMAGES_DESTINED  := $(addprefix $(DESTDIR)/$(RPK_TARGET).do-always-images/,$(filter $(IMAGES_SRCWILD),$(ALL_DONE)))

SOUNDS_SRC       := $(call FILTER_EXT,$(AUDIO_ALL_EXTS),$(ALL_FILES))
SOUNDS_SRCWILD   := $(call REPLACE_EXT,$(AUDIO_ALL_EXTS),$(SOUNDS_SRC))
SOUNDS_DESTINED  := $(addprefix $(DESTDIR)/$(RPK_TARGET).do-always-sounds/,$(filter $(SOUNDS_SRCWILD),$(ALL_DONE)))

FILES_RPK        := $(call FILTER_EXT,$(FILE_ALL_EXTS),$(ALL_FILES))
FILES_DESTINED   := $(addprefix $(DESTDIR)/$(RPK_TARGET).do-always-files/,$(FILES_RPK))
ifeq ($(PK3_PREFIX),xxx-multigame)
# I'm kind of amazed this works, I thought the path would be missing, build fail
FILES_DESTINED   += $(DESTDIR)/$(RPK_TARGET).do-always-files/sound/misc/silence.ogg
endif

REPACK_DESTINED  := $(subst .do-always-images,.do-always,$(IMAGES_DESTINED)) \
                    $(subst .do-always-sounds,.do-always,$(SOUNDS_DESTINED))

package: $(addprefix $(DESTDIR)/,$(TARGET_REPACK))
	$(eval RPK_TARGET := $(subst .do-always,.pk3,$(notdir $<)))
	$(echo_cmd) "PACKAGED $(RPK_TARGET)"
	-$(Q)$(MOVE) $(DESTDIR)/$(RPK_TARGET) $(DESTDIR)/$(RPK_TARGET).bak 2> /dev/null
	$(Q)$(MOVE) $(DESTDIR)/$(RPK_TARGET:.pk3=.zip) $(DESTDIR)/$(RPK_TARGET)
	-$(Q)$(UNLINK) $(DESTDIR)/$(RPK_TARGET).bak 2> /dev/null

$(DESTDIR)/$(RPK_TARGET).do-always/%:
	$(call DO_ARCHIVE,$(subst \space, ,$(subst .do-always,dir,$(call RPK_LOCAL,$@))),$(DESTDIR)/$(RPK_TARGET)dir,$(subst \space, ,$(PK3_PREFIX)).zip)

$(DESTDIR)/$(PK3_PREFIX).do-always: $(REPACK_DESTINED)
	@:


ifeq ($(SOUNDS_DESTINED),)
$(DESTDIR)/$(PK3_PREFIX)-sounds.do-always: $(SOUNDS_DESTINED)
	$(error no sounds for $(PK3_PREFIX)-sounds $(FILES_RPK), must be run on a pk3dir)
else
$(DESTDIR)/$(RPK_TARGET).do-always-sounds/%:
	$(call DO_ARCHIVE,$(subst $(DESTDIR)/$(RPK_TARGET).do-always-sounds/,,$@),$(SRCDIR)/,$(subst \space, ,$(PK3_PREFIX))-sounds.zip)

$(DESTDIR)/$(PK3_PREFIX)-sounds.do-always: $(SOUNDS_DESTINED)
	@:
endif

ifeq ($(IMAGES_DESTINED),)
$(DESTDIR)/$(PK3_PREFIX)-images.do-always: $(IMAGES_DESTINED)
	$(error no images for $(PK3_PREFIX)-images $(FILES_RPK), must be run on a pk3dir)
else
$(DESTDIR)/$(RPK_TARGET).do-always-images/%:
	$(call DO_ARCHIVE,$(subst $(DESTDIR)/$(RPK_TARGET).do-always-images/,,$@),$(SRCDIR)/,$(subst \space, ,$(PK3_PREFIX))-images.zip)

$(DESTDIR)/$(PK3_PREFIX)-images.do-always: $(IMAGES_DESTINED)
	@:
endif

ifeq ($(FILES_DESTINED),)
$(DESTDIR)/$(PK3_PREFIX)-files.do-always: $(FILES_DESTINED)
	$(error no files for $(PK3_PREFIX)-files $(FILES_RPK), must be run on a pk3dir)
else
$(DESTDIR)/$(RPK_TARGET).do-always-files/%:
	$(call DO_ARCHIVE,$(subst $(DESTDIR)/$(RPK_TARGET).do-always-files/,,$@),$(SRCDIR)/,$(subst \space, ,$(PK3_PREFIX))-files.zip)

$(DESTDIR)/$(PK3_PREFIX)-files.do-always: $(FILES_DESTINED)
	@:
endif

ifeq ($(QVM_DESTINED),)
$(error no vms, build game first)
else
$(DESTDIR)/$(RPK_TARGET).do-always-vms/%:
	$(call DO_ARCHIVE,vm/$(notdir $@),$(subst /vm,,$(subst \space, ,$(subst $(DESTDIR)/$(RPK_TARGET).do-always-vms/,,$(dir $@)))),$(subst \space, ,$(PK3_PREFIX))-vms.zip)

$(DESTDIR)/$(PK3_PREFIX)-vms.do-always: $(QVM_DESTINED)
	@:
endif

ifneq ($(filter $(MAKECMDGOALS),package-pk3dirs),)

$(DESTDIR)/%.do-always:
	+$(Q)$(MAKE) -f $(MKFILE) V=$(V) repack \
		SRCDIR="$(subst \space, ,$(call RPK_PK3DIR,$@))" \
		DESTDIR="$(DESTDIR)"

package-pk3dirs: $(addprefix $(DESTDIR)/,$(TARGET_REPACK))
	@:

endif

endif


################################################## build index file


ifdef TARGET_INDEX

WASM_ESSENTIAL   := gfx/2d/bigchars.png \
                    index.css $(WASM_VFS)
WASM_OBJS        := $(DESTDIR).do-always/$(WASM_HTML) \
                    $(addprefix $(DESTDIR).do-always/,$(WASM_ESSENTIAL)) \
                    $(DESTDIR)/quake3e.opt \
                    $(DESTDIR).do-always/quake3e.wasm

$(DESTDIR)/%.opt: $(DESTDIR)/%.wasm
	$(DO_OPT_CC)

# TODO: package a .js with loader code like emscripten does, for other libs
$(DESTDIR)/%.min.js: $(WASM_JS)
	$(DO_UGLY_CC)

$(DESTDIR).do-always/%.png: $(SRCDIR)/%.png
	$(DO_BASE64_CC)

$(DESTDIR).do-always/%.css: $(WASM_HTTP)/%.css
	$(DO_CSS_EMBED)

$(DESTDIR).do-always/%.pk3: build/%.pk3
	$(DO_ASSET_EMBED)

$(DESTDIR).do-always/%.wasm:
	$(DO_WASM_EMBED)

$(DESTDIR).do-always/%.js: $(DESTDIR)/%.min.js
	$(DO_JSBUILD_EMBED)

$(DESTDIR).do-always/$(WASM_HTML):
	$(call DO_INDEX_CC,$(DESTDIR)/$(WASM_HTML))

#$(DESTDIR).do-always/%.js: $(WASM_HTTP)/%.js
#	$(DO_JS_EMBED)

$(DESTDIR).do-always/quake3e.html: $(WASM_OBJS)
	$(DO_JS_LIST)
	-$(Q)$(MOVE) $(subst .do-always,,$@) $(subst .do-always,,$@).bak 2> /dev/null
	$(Q)$(MOVE) $(DESTDIR)/$(WASM_HTML) $(subst .do-always,,$@) 2> /dev/null
	-$(Q)$(UNLINK) $(subst .do-always,,$@).bak 2> /dev/null

index: $(addprefix $(DESTDIR).do-always/,$(TARGET_INDEX))
	@:

endif


############################################# MAIN / REPACK / INDEX / SYNC


ifeq ($(RPK_UNPACK),)

unpack:
	$(echo_cmd) "NOTHING TO UNPACK"

else
ifndef TARGET_UNPACK

unpack: ## unpack .pk3 zips into seperate .pk3dir folders
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) unpack \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_UNPACK="$(RPK_UNPACK)"

endif
endif


ifndef TARGET_MKDIRS

mkdirs:
	$(echo_cmd) "REPACK $(SRCDIR) -> $(RPK_WORKDIRS)"
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) mkdirs \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_MKDIRS="$(RPK_WORKDIRS)"

endif


ifndef TARGET_ENCODE

encode: mkdirs ## re-encode audio to ogg, for web
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) encode \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_ENCODE="$(PK3_PREFIX).do-always"

endif


ifndef TARGET_CONVERT

convert: mkdirs ## convert assets to web compatible format
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) convert \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_CONVERT="$(PK3_PREFIX).do-always"

endif


ifndef TARGET_REPACK

package: mkdirs convert encode ## compress converted assets back into a single .pk3 zip
	+$(Q)$(MAKE) -f $(MKFILE) V=$(V) package \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_REPACK="$(PK3_PREFIX).do-always"

endif


ifeq ($(RPK_PK3DIRS),)

package-pk3dirs:
	$(echo_cmd) "NOTHING TO PACKAGE"

else
ifndef TARGET_REPACK

repack: mkdirs convert encode ## collect and repackage an existing pk3/pk3dir
	$(echo_cmd) "DESCENDING $(RPK_PK3DIRS)"
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) package-pk3dirs \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_REPACK="$(subst .pk3,.do-always,$(RPK_PK3DIRS))"

endif
endif


ifeq ($(RPK_INDEXES),)

index:
	$(echo_cmd) "EMPTY INDEX, BUILD FIRST"

else
ifndef TARGET_INDEX

index: ## build an $(WASM_HTML) page out of built sources
	$(echo_cmd) "CREATING INDEX $(RPK_INDEXES)"
	+$(Q)$(MAKE) -f $(MKFILE) V=$(V) index \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_INDEX="$(RPK_INDEXES:.wasm=.html)"

endif
endif



sync: ## synchronize a local copy of all lvlworld content
	@:


help:
	@echo Please see docs: https://github.com/briancullinan/planet_quake/blob/master/docs/make.md
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n $(subst \space, ,$(addsuffix \n,$(MAKES))) \033[36m\033[0m\n"} /^${subst (|,(,$(HELPFILTER)}[a-zA-Z0-9_-]*:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MKFILE)

.NOTPARALLEL: index
.DEFAULT_GOAL := help
