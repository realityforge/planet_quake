HAVE_VM_COMPILED := false
BUILD_CLIENT     ?= 0
BUILD_SERVER     := 0
BUILD_STANDALONE := 1
USE_RENDERER_DLOPEN := 0
USE_SYSTEM_JPEG  := 0
USE_INTERNAL_JPEG := 0
USE_SYSTEM_LIBC  := 0
USE_ABS_MOUSE    := 1
USE_LOCAL_DED    := 0
USE_LAZY_LOAD    := 1
USE_LAZY_MEMORY  := 1
USE_MASTER_LAN   := 1
USE_CURL         := 0
USE_SDL          := 0
USE_IPV6         := 0
USE_OPENGL2      := 1
USE_VULKAN       := 0
NO_MAKE_LOCAL    := 1

include make/configure.make

NODE             := node
UGLIFY           := uglifyjs
COPY             := cp
UNLINK           := rm
MOVE             := mv
OPT              := wasm-opt
LD               := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/wasm-ld
CC               := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang
CXX              := libs/$(COMPILE_PLATFORM)/wasi-sdk-14.0/bin/clang++
BINEXT           := .wasm

SHLIBEXT         := wasm
SHLIBCFLAGS      := 
SHLIBLDFLAGS     := -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                    -Wl,--export-dynamic --no-standard-libraries \
                    -Wl,--no-entry -Wl,--allow-undefined-file=code/wasm/wasm.syms 

CLIENT_LDFLAGS   := -Wl,--import-memory -Wl,--import-table -Wl,--error-limit=200 \
                    -Wl,--no-entry --no-standard-libraries

RELEASE_LDFLAGS  := -Wl,--export-dynamic
# -Wl,--strip-all 
DEBUG_LDFLAGS    := -fvisibility=default -fno-inline -Wl,--export-dynamic

ifeq ($(BUILD_CLIENT),1)
SHLIBLDFLAGS     += -Wl,--allow-undefined-file=code/wasm/wasm.syms
CLIENT_LDFLAGS   += -Wl,--allow-undefined-file=code/wasm/wasm.syms
endif

ifeq ($(BUILD_RENDERER_OPENGL),1)
SHLIBLDFLAGS     += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
CLIENT_LDFLAGS   += -Wl,--allow-undefined-file=code/wasm/wasm-nogl.syms
endif

CLIENT_LDFLAGS   += code/wasm/wasi/libclang_rt.builtins-wasm32.a

# -fno-common -ffreestanding -nostdinc --no-standard-libraries

BASE_CFLAGS      += -Wall --target=wasm32 \
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


DEBUG_CFLAGS     := -fvisibility=default -fno-inline \
                    -DDEBUG -D_DEBUG -g -g3 -fPIC -gdwarf -gfull

RELEASE_CFLAGS   := -fvisibility=hidden \
                    -DNDEBUG -Ofast -O3 -Oz -fPIC -ffast-math
                    # -flto 

export INCLUDE   := -Icode/wasm/include 


ifdef B

WASM_MIN         := quake3e.min.js
WASM_HTTP        := code/wasm/http
WASM_OBJS        := quake3e.js sys_emgl.js sys_fs.js sys_in.js \
                    sys_net.js sys_std.js sys_wasm.js nipplejs.js
WASM_JS          := $(addprefix $(WASM_HTTP)/,$(notdir $(WASM_OBJS)))
WASM_ASSETS      := games/multigame/assets
WASM_IMG_ASSETS  := gfx/2d/bigchars.png
WASM_VFS         := $(addprefix $(WASM_ASSETS)/,$(WASM_IMG_ASSETS))
WASM_INDEX       := $(B)/$(TARGET) $(B)/$(TARGET).opt $(WASM_HTTP)/$(WASM_MIN).ugly \
                    $(WASM_HTTP)/index.html $(B)/index.html \
                    $(WASM_VFS)
WASM_VFSOBJ      := $(addprefix $(B)/assets/,$(WASM_IMG_ASSETS)) \
                    $(B)/assets/index.css $(B)/assets/$(WASM_MIN) \
                    $(B)/assets/$(TARGET)


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
	$(Q)$(NODE) -e "let b64encoded=fs.readFileSync('$<', 'base64');fs.writeFileSync('$(B)/index.html', fs.readFileSync('$(B)/index.html').toString('utf-8').replace('<\!-- BEGIN VFS INCLUDES -->', '<\!-- BEGIN VFS INCLUDES -->\n<img title=\"${subst $(WASM_ASSETS)/,,$<}\" src=\"data:image/png;base64,'+b64encoded+'\" />'))"
endef

define DO_CSS_EMBED
	$(echo_cmd) "CSS_EMBED $<"
	$(Q)$(NODE) -e "fs.writeFileSync('$(B)/index.html', fs.readFileSync('$(B)/index.html').toString('utf-8').replace('<link rel=\"stylesheet\" href=\"${subst $(WASM_HTTP)/,,$<}\" />', '<style type=\"text/css\">\n/* <\!-- ${subst $(WASM_ASSETS)/,,$<} */\n'+fs.readFileSync('$<', 'utf-8')+'\n/* --> */\n</style>'))"
endef

define DO_JS_EMBED
	$(echo_cmd) "JS_EMBED $<"
	$(Q)$(NODE) -e "fs.writeFileSync('$(B)/index.html', fs.readFileSync('$(B)/index.html').toString('utf-8').replace('<script async type=\"text/javascript\" src=\"${subst $(WASM_HTTP)/,,$<}\"></script>', '<script async type=\"text/javascript\">\n/* <\!-- ${subst $(WASM_ASSETS)/,,$<} */\n'+fs.readFileSync('$<', 'utf-8')+'\n/* --> */\n</script>'))"
endef

define DO_WASM_EMBED
	$(echo_cmd) "WASM_EMBED $<"
	$(Q)$(NODE) -e "fs.writeFileSync('$(B)/index.html', fs.readFileSync('$(B)/index.html').toString('utf-8').replace('</body>', '</body>\n<script async type=\"text/javascript\">\n/* <\!-- ${subst $(B)/,,$<} */\nwindow.programBytes=atob(\\''+fs.readFileSync('$<', 'base64')+'\\')\n/* --> */\n</script>'))"
endef


define DO_INDEX_CC
	$(echo_cmd) "INDEX_CC $<"
	$(Q)$(COPY) $(WASM_HTTP)/index.html $(B)/index.html
	$(Q)$(MAKE) -f $(MKFILE) B=$(B) V=$(V) $(B)/index.html.pak
endef

$(B)/$(TARGET).opt: $(B)/$(TARGET)
	$(DO_OPT_CC)

$(WASM_HTTP)/$(WASM_MIN).ugly: $(WASM_JS)
	$(DO_UGLY_CC)

$(B)/index.html: $(WASM_INDEX)
	$(DO_INDEX_CC)

$(B)/index.html.pak: $(WASM_VFSOBJ)
	@:

$(B)/assets/%: $(WASM_ASSETS)/%
	$(DO_BASE64_CC)

$(B)/assets/%.css: $(WASM_HTTP)/%.css
	$(DO_CSS_EMBED)

$(B)/assets/%.js: $(WASM_HTTP)/%.js
	$(DO_JS_EMBED)

$(B)/assets/%.wasm: $(B)/%.wasm
	$(DO_WASM_EMBED)

pre-build:
	@:

ifeq ($(B),$(BD))
post-build:
	@:

else
post-build:
	$(echo_cmd) "PACKING $(TARGET_CLIENT)"
	@$(MAKE) -f $(MKFILE) B=$(B) V=$(V) $(B)/index.html

#	
#	new Buffer(dataUrl, 'base64');

endif

# TODO: compile all js files into one/minify/webpack
# TODO: insert bigchars font into index page, insert all javascript and wasm into index page
# TODO: deploy index page with Actions

endif