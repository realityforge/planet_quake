
# most .make files are the format 
# 1) PLATFORM
# 2) BUILD OBJS
# 3) DEFINES
# 4) GOALS
#this make file adds an additional BUILD OBJS and defined down below

# .do-always is used to force a command to run, even if the target files already exist
#   this is for things like putting all the images and js files inside quake3e.html

ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif
_                 = $() $()

ifndef SRCDIR
SRCDIR           := games/multigame/assets
endif

ifndef PK3_PREFIX
PK3_PREFIX       := $(subst $(_),\space,$(subst .pk3dir,,$(notdir $(SRCDIR))))
endif

ifeq ($(SRCDIR),games/multigame/assets)
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
RPK_GOAL         := $(DESTDIR)/$(RPK_TARGET)
RPK_DOALWAYS     := $(subst .pk3,.do-always,$(RPK_TARGET))
#RPK_UNPACK      := $(subst \pathsep,,$(subst \space\pathsep, ,$(subst $(_),\space,$(subst $(SRCDIR)/,\pathsep,$(wildcard $(SRCDIR)/*.pk3)))))
RPK_PK3DIRS      := $(subst .pk3dir,.pk3,$(subst \pathsep,,$(subst \space\pathsep, ,$(subst $(_),\space,$(subst $(SRCDIR)/,\pathsep,$(wildcard $(SRCDIR)/*.pk3dir))))))
RPK_TARGETS      := $(RPK_TARGET) $(RPK_UNPACK) $(RPK_PK3DIRS)
RPK_WORKDIRS     := $(addsuffix dir,$(RPK_TARGETS))
RPK_REPLACE       = $(subst \space, ,$(subst $(RPK_GOAL)dir/,$(SRCDIR)/,$1))
RPK_LOCAL         = $(subst \space, ,$(subst $(RPK_GOAL).do-always/,,$1))
RPK_COPY          = $(subst \space, ,$(subst $(DESTDIR)/$(PK3_PREFIX).do-always/,$(SRCDIR)/,$1))
RPK_COPIED        = $(subst \space, ,$(subst $(DESTDIR)/$(PK3_PREFIX).do-always/,$(RPK_GOAL)dir/,$1))
RPK_GETEXT        = $(basename $1).$(call GETEXT,$(call RPK_REPLACE,$1))
RPK_PK3DIR        = $(subst $(DESTDIR)/,$(SRCDIR)/,$(subst .do-always,.pk3dir,$1))
RPK_INDEXES      += $(notdir $(wildcard build/*/$(CNAME)*.wasm))


WASM_HTML        := index.html
WASM_HTTP        := code/wasm/http
WASM_FILES       := $(CNAME).js sys_emgl.js sys_fs.js sys_in.js \
                    sys_net.js sys_std.js sys_wasm.js sys_snd.js nipplejs.js
WASM_JS          := $(addprefix $(WASM_HTTP)/,$(notdir $(WASM_FILES)))
WASM_ASSETS      := games/multigame/assets

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

ifneq ($(filter %.pk3dir,$(SRCDIR)),)

# must convert files in 2 seperate steps because of images 
#   we don't know what format we end up with until after the
#   conversion so we don't repeat the the GETEXT command excessively
FILES_SRCPK3     := $(call LEVELS_PK3,$(subst \space, ,$(SRCDIR)))
FILES_SRC        := $(call LEVELS,$(subst \space, ,$(SRCDIR)))
ALL_FILES        := $(filter-out $(FILES_SRCPK3),$(FILES_SRC))
FILES_DONEPK3    := $(call LEVELS_PK3,$(subst \space, ,$(RPK_GOAL)dir))
FILES_DONE       := $(call LEVELS,$(subst \space, ,$(RPK_GOAL)dir))
ALL_DONE         := $(filter-out $(FILES_DONEPK3),$(FILES_DONE))


ifeq ($(filter-out $(_),$(ALL_FILES)),)
$(error no files in source directory $(FILES_SRC))
endif

endif

IMAGE_VALID_EXTS := jpg png
IMAGE_CONV_EXTS  := dds tga bmp pcx
IMAGE_ALL_EXTS   := $(IMAGE_CONV_EXTS) $(IMAGE_VALID_EXTS)
AUDIO_VALID_EXTS := ogg
AUDIO_CONV_EXTS  := wav mp3 opus mpga
AUDIO_ALL_EXTS   := $(AUDIO_CONV_EXTS) $(AUDIO_VALID_EXTS)
FILE_ALL_EXT     := cfg skin menu shaderx mtr arena bot txt shader




################################################# DO WORK DEFINES

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
	$(echo_cmd) "ARCHIVE $@"
	$(Q)pushd "$(RPK_GOAL)dir" > /dev/null && \
	$(ZIP) -o ../$(subst \space, ,$(PK3_PREFIX)).zip "$(subst \space, ,$(subst .do-always,dir,$(call RPK_LOCAL,$@)))" > /dev/null && \
	popd > /dev/null
endef


########################################################### wasm indexing


define DO_OPT_CC
	$(echo_cmd) "OPT_CC $<"
	$(Q)$(OPT) -Os --no-validation -o $@ $<
	-$(Q)$(MOVE) $< $<.bak > /dev/null
	$(Q)$(MOVE) $@ $< > /dev/null
	-$(Q)$(UNLINK) $<.bak > /dev/null
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
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_VFSHEAD)','$(HTML_VFSHEAD)\n<img title=\"$(notdir $<)\" src=\"$(call WASM_BASE64,$<)\" />',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_CSS_EMBED
	$(echo_cmd) "CSS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(call HTML_LINK,$(subst $(WASM_HTTP)/,,$<))','$(call HTML_STYLE,$(subst $(WASM_ASSETS)/,,$<),$<)',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_JS_LIST
	$(echo_cmd) "JS_LIST $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(call HTML_SCRIPT,$(CNAME).js)','$(WASM_FILES)'.split(' ').map(jsFile => '$(call JS_SCRIPT,'+$(call NODE_FSREAD,$(WASM_HTTP)/'+jsFile+')+','+jsFile+')').join(''),$(DESTDIR)/$(WASM_HTML))"
endef

define DO_JSBUILD_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'<script async type=\"text/javascript\" src=\"${subst $(DESTDIR)/,,$<}\"></script>','$(call JS_SCRIPT,'+fs.readFileSync('$<', 'utf-8')+',$(subst $(DESTDIR)/,,$<))',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_JS_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "fs.writeFileSync('$(DESTDIR)/$(WASM_HTML)', fs.readFileSync('$(DESTDIR)/$(WASM_HTML)').toString('utf-8').replace('<script async type=\"text/javascript\" src=\"${subst $(WASM_HTTP)/,,$<}\"></script>', '$(call JS_SCRIPT,'+fs.readFileSync('$<', 'utf-8')+',$(subst $(WASM_HTTP)/,,$<))'))"
endef

define DO_WASM_EMBED
	$(echo_cmd) "WASM_EMBED $@"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n$(call JS_SCRIPT,$(call JS_PREFS,$(<:.opt=.wasm),$(CNAME).wasm),$(subst $(DESTDIR)/,,$<))',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_ASSET_EMBED
	$(echo_cmd) "ASSET_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n<script async type=\"text/javascript\">\n/* <\!-- ${subst $(DESTDIR)/,,$<} */\nwindow.preFS[\\'multigame/${notdir $<}\\']=\\''+fs.readFileSync('$<', 'base64')+'\\'\n/* --> */\n</script>',$(DESTDIR)/$(WASM_HTML))"
endef

define DO_INDEX_CC
	$(echo_cmd) "INDEX_CC $@"
	$(Q)$(COPY) $(WASM_HTTP)/$(WASM_HTML) $(subst .do-always,,$@)
	$(Q)$(NODE) -e "console.assert('$(subst .do-always,,$@)' == '$(DESTDIR)/$(WASM_HTML)');$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n<script async type=\"text/javascript\">\nwindow.preStart=[\\'$(STARTUP_COMMAND)\\'];\n/* --> */\n</script>',$(DESTDIR)/$(WASM_HTML))"
endef


######################################################### TARGETS / GOALS


#################################################### image conversion

ifdef TARGET_CONVERT

# list images with converted pathname then check for existing alt-name in 
#   defined script
# convert jpg from source dirs in case there is a quality conversion
IMAGE_SRC        := $(call FILTER_EXT,$(IMAGE_ALL_EXTS),$(ALL_FILES))
IMAGE_DONE       := $(call FILTER_EXT,$(IMAGE_VALID_EXTS),$(ALL_DONE))
IMAGE_DONE_WILD  := $(call REPLACE_EXT,$(IMAGE_VALID_EXTS),$(IMAGE_DONE))
IMAGE_NEEDED     := $(filter-out $(IMAGE_DONE_WILD),$(IMAGE_SRC))
IMAGE_OBJS       := $(addprefix $(RPK_GOAL)dir/,$(IMAGE_NEEDED))

convert: $(addprefix $(DESTDIR)/,$(TARGET_CONVERT))
	@:

$(RPK_GOAL)dir/%.tga:
	$(NODE) -e "let wrong=fs.readFileSync('$(call RPK_REPLACE,$@)', 'binary');if(wrong.includes('created using ImageLib by SkyLine Tools')){wrong=wrong.replace('created using ImageLib by SkyLine Tools', '');wrong=wrong.replace(/^\(/, '\0');fs.writeFileSync('$(call RPK_REPLACE,$@)', wrong)}"
	$(DO_CONVERT)

$(RPK_GOAL)dir/%:
	$(DO_CONVERT)

ifeq ($(IMAGE_OBJS),)

$(DESTDIR)/$(PK3_PREFIX).do-always:
	$(echo_cmd) "NOTHING TO CONVERT"

else

$(DESTDIR)/$(PK3_PREFIX).do-always: $(IMAGE_OBJS)
	$(echo_cmd) "CONVERTED $<"

endif

endif # TARGET_CONVERT

ifeq ($(TARGET_CONVERT),)

convert:
	$(echo_cmd) "NOTHING TO CONVERT"

endif



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
AUDIO_OBJS       := $(addprefix $(RPK_GOAL)dir/,$(AUDIO_NEEDED))

encode: $(addprefix $(DESTDIR)/,$(TARGET_ENCODE))
	@:

$(RPK_GOAL)dir/%.mp3:
	$(DO_FFMPEG)

$(RPK_GOAL)dir/%.mpga:
	$(DO_FFMPEG)

$(RPK_GOAL)dir/%.wav:
	$(if $(call GETMPGA,$(call RPK_REPLACE,$@)),$(DO_ENCODE),$(DO_FFMPEG))

$(RPK_GOAL)dir/%:
	$(DO_ENCODE)

$(DESTDIR)/$(PK3_PREFIX).do-always/%:
	$(DO_COPY)

$(DESTDIR)/$(PK3_PREFIX).do-always: $(AUDIO_OBJS) $(AUDIO_SRCDONE)
	$(echo_cmd) "ENCODED $<"

endif # TARGET_ENCODE

ifeq ($(TARGET_ENCODE),)

encode:
	$(echo_cmd) "NOTHING TO ENCODE"

endif




################################################# pk3s extraction


ifdef TARGET_UNPACK

unpack: $(TARGET_UNPACK)
	$(echo_cmd) "UNPACKED $<"

# have to do this first and it runs with no replace 
#   so it's not expensive to repeat every time
$(DESTDIR)/%.pk3: $(SRCDIR)/%.pk3
	$(DO_UNPACK)

endif


ifeq ($(TARGET_UNPACK),)

unpack:
	$(echo_cmd) "NOTHING TO UNPACK"

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

IMAGE_SRC        := $(call FILTER_EXT,$(IMAGE_ALL_EXTS),$(ALL_FILES))
IMAGE_SRCWILD    := $(call REPLACE_EXT,$(IMAGE_ALL_EXTS),$(IMAGE_SRC))
IMAGE_DESTINED   := $(addprefix $(RPK_GOAL).do-always/,$(filter $(IMAGE_SRCWILD),$(ALL_DONE)))

$(DESTDIR)/%.do-always:
	+$(Q)$(MAKE) -f $(MKFILE) V=$(V) repack \
		SRCDIR="$(subst \space, ,$(call RPK_PK3DIR,$@))" DESTDIR="$(DESTDIR)"

package-pk3dirs: $(addprefix $(DESTDIR)/,$(TARGET_REPACK))
	@:

package: $(addprefix $(DESTDIR)/,$(TARGET_REPACK))
	$(echo_cmd) "PACKAGED $<"
	-@$(MOVE) $(RPK_GOAL) $(RPK_GOAL).bak > /dev/null
	$(Q)$(MOVE) $(DESTDIR)/$(PK3_PREFIX).zip $(RPK_GOAL)
	-@$(UNLINK) $(RPK_GOAL).bak > /dev/null

$(RPK_GOAL).do-always/%:
	$(DO_ARCHIVE)

$(DESTDIR)/$(PK3_PREFIX).do-always: $(IMAGE_DESTINED)
	@:

endif

ifeq ($(TARGET_REPACK),)

package-pk3dirs:
	$(echo_cmd) "NOTHING TO PACKAGE"

endif



################################################## build index file


ifdef TARGET_INDEX

WASM_VFSOBJ      := gfx/2d/bigchars.png \
                    index.css
WASM_OBJS        := $(DESTDIR).do-always/$(WASM_HTML) \
										$(addprefix $(DESTDIR)/,$(WASM_VFSOBJ)) \
										$(DESTDIR)/quake3e.opt \
										$(DESTDIR).do-always/quake3e.wasm

$(DESTDIR).do-always/quake3e.html: $(WASM_OBJS)
	$(DO_JS_LIST)
	-$(Q)$(MOVE) $@ $@.bak > /dev/null
	$(Q)$(MOVE) $(DESTDIR)/$(WASM_HTML) $@ > /dev/null
	-$(Q)$(UNLINK) $@.bak > /dev/null

$(DESTDIR).do-always/$(WASM_HTML):
	$(call DO_INDEX_CC,$(DESTDIR)/$(WASM_HTML))

$(DESTDIR)/%.opt: $(DESTDIR)/%.wasm
	$(DO_OPT_CC)

$(DESTDIR)/%.min.js: $(WASM_JS)
	$(DO_UGLY_CC)

$(DESTDIR)/%.png: $(SRCDIR)/%.png
	$(DO_BASE64_CC)

$(DESTDIR)/%.css: $(WASM_HTTP)/%.css
	$(DO_CSS_EMBED)

$(DESTDIR).do-always/%.wasm:
	$(DO_WASM_EMBED)

$(DESTDIR).do-always/%.js: $(DESTDIR)/%.min.js
	$(DO_JSBUILD_EMBED)

#$(DESTDIR).do-always/%.js: $(WASM_HTTP)/%.js
#	$(DO_JS_EMBED)

$(DESTDIR).do-always/%.pk3: $(DESTDIR)/%.pk3
	$(DO_ASSET_EMBED)

index: $(addprefix $(DESTDIR).do-always/,$(TARGET_INDEX))
	@:

endif



######################################## MAIN / REPACK / INDEX / SYNC

repack: ## repackage an existing pk3/pk3dir
	$(echo_cmd) "REPACK $(SRCDIR) -> $(RPK_WORKDIRS)"
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) mkdirs \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_MKDIRS="$(RPK_WORKDIRS)"
#	$(Q)$(MAKE) -f $(MKFILE) V=$(V) unpack \
#		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
#		TARGET_UNPACK="$(addprefix $(DESTDIR)/,$(RPK_UNPACK))"
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) convert \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_CONVERT="$(RPK_CONVERT)"
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) encode \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_ENCODE="$(RPK_ENCODE)"
#	+$(Q)$(MAKE) -f $(MKFILE) V=$(V) package \
#		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
#		TARGET_REPACK="$(DESTDIR)/$(PK3_PREFIX).do-always"
	$(Q)$(MAKE) -f $(MKFILE) V=$(V) package-pk3dirs \
		SRCDIR="$(SRCDIR)" DESTDIR="$(DESTDIR)" \
		TARGET_REPACK="$(subst .pk3,.do-always,$(RPK_PK3DIRS))"


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
