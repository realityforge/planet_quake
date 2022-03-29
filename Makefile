MKFILE           := $(lastword $(MAKEFILE_LIST))
HELPFILTER       ?= 
_                := $() $()

HELP_CMDS        := help build platform games
HELP_TARGETS     := $(filter $(MAKECMDGOALS),$(HELP_CMDS))

PLATFORMS        := darwin linux win wasm
PLATFORM_TARGETS := $(filter $(MAKECMDGOALS),$(PLATFORMS))

BUILD_POSTOP     := 
BUILD_PREOP      := 
BUILD_MODE       := debug
BUILD_MODES      := debug release clean deploy install
MODE_TARGETS     := $(filter $(MAKECMDGOALS),$(BUILD_MODES))
ENGINE_FEATURES  := experimental mw static
RENDER_FEATURES  := -vulkan -mw 2 2-mw
BUILD_FEATURES   := client $(addprefix client-,$(ENGINE_FEATURES)) \
										server $(addprefix server-,$(ENGINE_FEATURES)) \
										renderer $(addprefix renderer,$(RENDER_FEATURES)) \
										huffman 
BUILD_CMD        := 
BUILD_TARGETS    := $(filter $(MAKECMDGOALS),$(BUILD_FEATURES) $(ENGINE_FEATURES))
BUILD_PLATFORM   := 
BUILD_MAKEFILE   := 
CLIENT_OPTIONS   := 
LIB_OPTIONS      := 

INSTALL_DIR      ?= bin/



all: ## build everything the entire system can build
	@$(MAKE) -f $(MKFILE) -j 16 \
		debug 

build: ## build the engine code
	$(eval HELPFILTER += client server renderer renderer2 server libs games)
	@:

client: ## build the client program for this system
	$(eval BUILD_MAKEFILE += make/build_client.make)
	$(eval HELPFILTER += client)
	@:

experimental: ## build drag and drop and persistent console
	$(eval CLIENT_OPTIONS += BUILD_EXPERIMENTAL=1)
	@:

static: ## build drag and drop and persistent console
	$(eval CLIENT_OPTIONS += BUILD_CLIENT_STATIC=1)
	@:

mw: multiworld ## (alias) build drag and drop and persistent console
	@:

multiworld: ## build drag and drop and persistent console
	$(eval HELPFILTER += mw multiworld)
	$(eval CLIENT_OPTIONS += USE_MULTIVM_CLIENT=1 USE_MULTIVM_SERVER=1)
	@:

client-experimental: ## build drag and drop and persistent console
	$(eval CLIENT_OPTIONS += BUILD_EXPERIMENTAL=1)
	@:

client-mw: client mw ## build multiworld features and multiple VMs
	@:

client-static: client static ## build all libraries statically
	@:

huffman: ## build huffman decoder library for sniffing
	$(eval DIDWORK=1)
	@:

renderer: ## build the OpenGL renderer as a dynamic library
	$(eval BUILD_MAKEFILE += make/build_renderer.make)
	@:

renderer2: ## build the OpenGL 2.5 renderer as a dynamic library
	$(eval BUILD_MAKEFILE += make/build_renderer2.make)
	@:

renderer-mw: renderer mw ## build dynamic renderer with multiworld, lazy-loading
	@:

renderer2-mw: renderer 2 mw ## build dynamic renderer with multiworld, lazy-loading
	@:


server: ## build the dedicated server standalone
	$(eval BUILD_MAKEFILE += make/build_server.make)
	@:

server-experimental: server experimental ## build dedicated with experimental features
	@:

server-mw: server mw ## build dedicated with multiworld features
	@:

games: ## make some cool QVMs
	$(eval HELPFILTER += baseq3a ioq3 multi tremulous oa)
	@:

baseq3a: ## make baseq3a, a bug fixed version of 1.32e, works with MISSIONPACK=1
	$(eval HELPFILTER += games baseq3a multiworld)
	$(eval BUILD_MAKEFILE += make/game_baseq3a.make)
	@:

baseq3a-static: ## build static baseq3a into engine exe
	$(eval BUILD_MAKEFILE += make/build_client.make)
	$(eval CLIENT_OPTIONS += BUILD_GAME_STATIC=1 BUILD_BASEQ3A=1)
	@:

multigame: ## make multigame, a portal enhanced, configurable mod
	$(eval BUILD_MAKEFILE += make/game_multi.make)
	@:

multigame-static: ## build static multigame into engine exe
	$(eval BUILD_MAKEFILE += make/build_client.make)
	$(eval CLIENT_OPTIONS += BUILD_GAME_STATIC=1 BUILD_MULTIGAME=1)
	@:

libs: ## make some libraries that the engine needs
	$(eval HELPFILTER += botlib curl freetype glib huffman musl opus png jpeg mbspc rmlui vorbis webm vpx zlib)
	@:

botlib: ## build bots as a dynamic lib so AI can be swapped out for different intelligences
	$(eval LIB_OPTIONS += USE_BOTLIB_DLOPEN=1)
	$(eval CLIENT_OPTIONS += USE_BOTLIB_DLOPEN=1)
	$(eval BUILD_MAKEFILE += make/lib_botlib.make)
	@:

curl: ## build cURL download library for client downloads USE_CURL=1
	$(eval BUILD_MAKEFILE += make/lib_curl.make)
	$(eval CLIENT_OPTIONS += USE_CURL=1)
	@:

rmlui: ## an attempt to use RMLUI as a user interface
	$(eval BUILD_MAKEFILE += make/lib_rmlui.make)
	$(eval CLIENT_OPTIONS += USE_RMLUI=1)
	@:

jpeg: ## build jpeg 9 from source
	$(eval BUILD_MAKEFILE += make/lib_jpeg.make)
	$(eval CLIENT_OPTIONS += USE_SYSTEM_JPEG=0)
	@:

vorbis: ## build ogg-vorbis from source for embedding in WASM
	$(eval BUILD_MAKEFILE += make/lib_vorbis.make)
	$(eval CLIENT_OPTIONS += USE_SYSTEM_VORBIS=0)
	@:

platform: ## support cross-compiling to other/embedded platforms like win,unix,wasm,macos
	$(eval HELPFILTER += darwin linux unix wasm mingw)
	@:


ifneq ($(PLATFORM),wasm)
wasm: ## cross compile wasm support
	$(eval BUILD_PLATFORM = PLATFORM=wasm)
	@:
else
wasm:
	@:
endif

mingw: ## cross compile for windows with ming
	@:

clean: ## clean the project object files
	$(eval HELPFILTER += release clean debug)
	$(eval BUILD_MODE += clean)
	@:

deploy: ## deploy necessary build files to a cloud bucket
	$(eval HELPFILTER += release deploy)
	$(eval BUILD_MODE += deploy)
	@:

release: ## build release mode
	$(eval HELPFILTER += release clean deploy)
	$(eval BUILD_MODE += release)
	@:

debug: ## build debug mode
	$(eval HELPFILTER += debug clean)
	$(eval BUILD_MODE += debug)
	@:

install: ## install the build programs into the INSTALL_DIR
	$(eval HELPFILTER += release install deploy)
	$(eval BUILD_MODE += install)
	$(eval CLIENT_OPTIONS += INSTALL_DIR=$(INSTALL_DIR))
	@:

eval-commands: 
	$(eval HELPFILTER = ($(subst ||,,$(subst $(_),|,$(HELPFILTER)))))
	$(eval COMMANDS = $(BUILD_MODE) $(BUILD_PLATFORM) $(BUILD_CMD) $(CLIENT_OPTIONS))
	$(eval MAKES = $(addprefix make\space-f\space,$(BUILD_MAKEFILE)))
	$(eval MAKES = $(addsuffix $(subst $(_),\space, $(COMMANDS)),$(MAKES)))

ifeq ($(filter $(MAKECMDGOALS),make),make)

make: $(HELP_TARGETS) $(MODE_TARGETS) $(PLATFORM_TARGETS) $(BUILD_TARGETS) eval-commands
	@echo "MAKING $(subst \space, ,$(addsuffix \n,$(MAKES)))"
	$(subst \space, ,$(addsuffix \n,$(MAKES)))

else

%: $(HELP_TARGETS) $(MODE_TARGETS) $(PLATFORM_TARGETS) $(BUILD_TARGETS) help 
	@:

endif

help: eval-commands ## print help docs in Makefile
	@echo Please see docs: https://github.com/briancullinan/planet_quake/blob/master/docs/make.md
	awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n $(subst \space, ,$(addsuffix \n,$(MAKES))) \033[36m\033[0m\n"} /^${subst (|,(,$(HELPFILTER)}[a-zA-Z0-9_-]*:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MKFILE)


.DEFAULT_GOAL := help
.PHONY: help build release debug client libs platform games jpeg rmlui vorbis multigame baseq3a server renderer renderer2 huffman wasm client-static renderer-static renderer2-static client-experimental server-experimental server-mw client-mw all install experimental multiworld mw static clean deploy install make
#.RECIPEPREFIX +=
