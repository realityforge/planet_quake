HAVE_VM_COMPILED    := false
BUILD_CLIENT        ?= 0
BUILD_SERVER        := 0
BUILD_STANDALONE    := 1
USE_RENDERER_DLOPEN := 0
USE_SYSTEM_JPEG     := 0
USE_INTERNAL_JPEG   := 0
USE_INTERNAL_VORBIS := 1
USE_SYSTEM_LIBC     := 0
USE_CODEC_VORBIS    := 1
USE_CODEC_WAV       := 0
USE_ABS_MOUSE       := 1
USE_LOCAL_DED       := 0
USE_LAZY_LOAD       := 1
USE_LAZY_MEMORY     := 1
USE_MASTER_LAN      := 1
USE_CURL            := 0
USE_SDL             := 0
USE_IPV6            := 0
USE_OPENGL2         := 1
USE_VULKAN          := 0
NO_MAKE_LOCAL       := 1

include make/configure.make

INSTALLS            += quake3e.html
NODE                := node
UGLIFY              := uglifyjs
COPY                := cp
UNLINK              := rm
MOVE                := mv
OPT                 := wasm-opt
LD                  := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/wasm-ld
CC                  := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang
CXX                 := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang++
BINEXT              := .wasm

SHLIBEXT            := wasm
SHLIBCFLAGS         := -frtti -fPIC -MMD
SHLIBLDFLAGS        := -fPIC -Wl,-shared \
                       -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                       -Wl,--no-entry --no-standard-libraries -Wl,--export-dynamic
CLIENT_LDFLAGS      := -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                       -Wl,--no-entry --no-standard-libraries -Wl,--export-dynamic
RELEASE_LDFLAGS     := 
DEBUG_LDFLAGS       := -fvisibility=default -fno-inline

ifeq ($(BUILD_CLIENT),1)
SHLIBLDFLAGS        += -fvisibility=default -Wl,--allow-undefined-file=code/wasm/wasm.syms
CLIENT_LDFLAGS      += -Wl,--allow-undefined-file=code/wasm/wasm.syms
endif

ifeq ($(BUILD_RENDERER_OPENGL),1)
SHLIBLDFLAGS        += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
CLIENT_LDFLAGS      += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
endif

ifeq ($(BUILD_VORBIS),1)
SHLIBLDFLAGS        += -Wl,--allow-undefined-file=code/wasm/wasm-lib.syms
CLIENT_LDFLAGS      += -Wl,--allow-undefined-file=code/wasm/wasm-lib.syms
endif


ifndef BUILD_VORBIS
ifeq ($(USE_CODEC_VORBIS),1)
ifneq ($(USE_INTERNAL_VORBIS),1)
  CLIENT_LDFLAGS    += -L$(B) -lvorbis_$(ARCH)
endif
endif
endif

CLIENT_LDFLAGS      += code/wasm/wasi/libclang_rt.builtins-wasm32.a

# -fno-common -ffreestanding -nostdinc --no-standard-libraries

BASE_CFLAGS         += -Wall --target=wasm32 \
                       -Wimplicit -fstrict-aliasing \
                       -ftree-vectorize -fsigned-char -MMD \
                       -ffast-math -fno-short-enums \
                       -pedantic \
                       -Wno-extra-semi \
                       -DGL_GLEXT_PROTOTYPES=1 \
                       -DGL_ARB_ES2_compatibility=1 \
                       -DGL_EXT_direct_state_access=1 \
                       -DUSE_Q3KEY \
                       -DUSE_MD5 \
                       -DUSE_ABS_MOUSE \
                       -DUSE_LAZY_LOAD \
                       -DUSE_LAZY_MEMORY \
                       -DUSE_MASTER_LAN \
                       -D__WASM__ \
                       -std=c11 \
                       -Icode/wasm \
                       -Ilibs/musl-1.2.2/include

DEBUG_CFLAGS        := -fvisibility=default -fno-inline \
                       -DDEBUG -D_DEBUG -g -g3 -fPIC -gdwarf -gfull
RELEASE_CFLAGS      := -fvisibility=hidden \
                       -DNDEBUG -Ofast -O3 -Oz -fPIC -ffast-math
                    # -flto 
export INCLUDE      := -Icode/wasm/include 
STARTUP_COMMAND     := +devmap\\', \\'lsdm3_v1

ifdef B

PK3_INCLUDES     := xxx-multigame-files.pk3 \
                    xxx-multigame-vms.pk3 \
                    lsdm3_v1-files.pk3 \
                    lsdm3_v1-images.pk3 \
                    xxx-multigame-sounds.pk3


WASM_HTTP        := code/wasm/http
WASM_FILES       := $(CNAME).js sys_emgl.js sys_fs.js sys_in.js \
                    sys_net.js sys_std.js sys_wasm.js sys_snd.js nipplejs.js
WASM_JS          := $(addprefix $(WASM_HTTP)/,$(notdir $(WASM_FILES)))
WASM_ASSETS      := games/multigame/assets
WASM_VFSOBJ      := gfx/2d/bigchars.png \
                    index.css
WASM_OBJS        := $(addprefix $(B)/assets/,$(WASM_VFSOBJ))

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

define DO_OPT_CC
	$(echo_cmd) "OPT_CC $<"
	$(Q)$(OPT) -Os --no-validation -o $@ $<
	-$(Q)$(MOVE) $< $<.bak 
	$(Q)$(MOVE) $@ $< 
	-$(Q)$(UNLINK) $<.bak
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
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_VFSHEAD)','$(HTML_VFSHEAD)\n<img title=\"$(subst $(WASM_ASSETS)/,,$<)\" src=\"$(call WASM_BASE64,$<)\" />',$(B)/index.html)"
endef

define DO_CSS_EMBED
	$(echo_cmd) "CSS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(call HTML_LINK,$(subst $(WASM_HTTP)/,,$<))','$(call HTML_STYLE,$(subst $(WASM_ASSETS)/,,$<),$<)',$(B)/index.html)"
endef

define DO_JS_LIST
	$(echo_cmd) "JS_LIST $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(call HTML_SCRIPT,quake3e.min.js)','$(WASM_FILES)'.split(' ').map(jsFile => '$(call JS_SCRIPT,'+$(call NODE_FSREAD,$(WASM_HTTP)/'+jsFile+')+','+jsFile+')').join(''),$(B)/index.html)"
endef

define DO_JSBUILD_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'<script async type=\"text/javascript\" src=\"${subst $(B)/,,$<}\"></script>','<script async type=\"text/javascript\">\n/* <\!-- ${subst $(B)/,,$<} */\n'+fs.readFileSync('$<', 'utf-8')+'\n/* --> */\n</script>',$(B)/$(WASM_HTML))"
endef

define DO_JS_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "fs.writeFileSync('$(B)/$(WASM_HTML)', fs.readFileSync('$(B)/$(WASM_HTML)').toString('utf-8').replace('<script async type=\"text/javascript\" src=\"${subst $(WASM_HTTP)/,,$<}\"></script>', '<script async type=\"text/javascript\">\n/* <\!-- ${subst $(WASM_HTTP)/,,$<} */\n'+fs.readFileSync('$<', 'utf-8')+'\n/* --> */\n</script>'))"
endef

define DO_WASM_EMBED
	$(echo_cmd) "WASM_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n$(call JS_SCRIPT,$(call JS_PREFS,$(<:.opt=.wasm),$(CNAME).wasm),$(subst $(B)/,,$<))',$(B)/index.html)"
endef

define DO_ASSET_EMBED
	$(echo_cmd) "ASSET_EMBED $<"
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n<script async type=\"text/javascript\">\n/* <\!-- ${subst $(B)/,,$<} */\nwindow.preFS[\\'multigame/${notdir $<}\\']=\\''+fs.readFileSync('$<', 'base64')+'\\'\n/* --> */\n</script>',$(B)/index.html)"
endef

define DO_INDEX_CC
	$(echo_cmd) "INDEX_CC $@"
	$(COPY) -f $(WASM_HTTP)/index.html $(dir $(subst $(B)/assets/,$(B)/,$@))index.html
	$(Q)$(NODE) -e "$(call NODE_FSREPLACE,'$(HTML_BODY)','$(HTML_BODY)\n<script async type=\"text/javascript\">\nwindow.preStart=[\\'$(STARTUP_COMMAND)\\'];\n/* --> */\n</script>',$(B)/index.html)"
endef


$(B)/%.opt: $(B)/%.wasm
	$(DO_OPT_CC)

$(B)/%.min.js: $(WASM_JS)
	$(DO_UGLY_CC)

$(B)/assets/%.png: $(WASM_ASSETS)/%.png
	$(DO_BASE64_CC)

$(B)/assets/%.css: $(WASM_HTTP)/%.css
	$(DO_CSS_EMBED)

$(B)/assets/%.js: $(B)/%.min.js
	$(DO_JSBUILD_EMBED)

#$(B)/assets/%.js: $(WASM_HTTP)/%.js
#	$(DO_JS_EMBED)

$(B)/assets/%.wasm: $(B)/%.opt
	$(DO_WASM_EMBED)

$(B)/assets/%.pk3: $(B)/%.pk3
	$(DO_ASSET_EMBED)

$(B)/assets/%.html: $(WASM_HTTP)/index.html
	$(DO_INDEX_CC)

index: $(B)/assets/quake3e.html $(B)/assets/quake3e.wasm $(WASM_OBJS)
	$(DO_JS_LIST)
	-$(Q)$(MOVE) $(B)/quake3e.html $(B)/quake3e.html.bak 
	$(Q)$(MOVE) $(B)/index.html $(B)/quake3e.html 
	-$(Q)$(UNLINK) $(B)/quake3e.html.bak

.NOTPARALLEL: index
# TODO: compile all js files into one/minify/webpack
# TODO: insert bigchars font into index page, insert all javascript and wasm into index page
# TODO: deploy index page with Actions

endif