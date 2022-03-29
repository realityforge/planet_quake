MKFILE           := $(lastword $(MAKEFILE_LIST))
HELPFILTER       ?= [a-zA-Z0-9_-]+
_                := $() $()


build: ## build some things
	@$(MAKE) -f $(MKFILE) HELPFILTER="(client|server|renderer|renderer2|server)" help

client: ## build the client program for this system
	@:

huffman: ## build huffman decoder library for sniffing
	$(eval DIDWORK=1)
	@:

renderer: ## build the OpenGL renderer as a dynamic library
	@:

renderer2: ## build the OpenGL 2.5 renderer as a dynamic library
	@:


server: ## build the dedicated server standalone
	@:

games: ## make some cool QVMs
	@$(MAKE) -f $(MKFILE) HELPFILTER="(baseq3a|ioq3|multi|tremulous|oa)" help

baseq3a: ## make baseq3a, a bug fixed version of 1.32e, works with MISSIONPACK=1
	@:

multigame: ## make multigame, a portal enhanced, configurable mod
	@:

libs: ## make some libraries that the engine needs
	@$(MAKE) -f $(MKFILE) HELPFILTER="(botlib|curl|freetype|glib|huffman|musl|opus|png|jpeg|mbspc|rmlui|vorbis|webm|vpx|zlib)" help

botlib: ## build bots as a dynamic lib so AI can be swapped out for different intelligences
	@:

curl: ## build cURL download library for client downloads USE_CURL=1
	@:

rmlui: ## an attempt to use RMLUI as a user interface
	@:

jpeg: ## build jpeg 9 from source
	@:

vorbis: ## build ogg-vorbis from source for embedding in WASM
	@:

platform: ## support cross-compiling to other/embedded platforms like win,unix,wasm,macos
	@:

release: ## build release mode
	@:

debug: ## build debug mode
	@:

help: ## print help docs in Makefile
	@echo Please see docs: https://github.com/briancullinan/planet_quake/blob/master/docs/make.md
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n make \033[36m\033[0m\n"} /^$(HELPFILTER):.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MKFILE)


.DEFAULT_GOAL := help
.PHONY: help build client libs platform games jpeg rmlui vorbis multigame baseq3a server renderer renderer2 huffman
#.RECIPEPREFIX +=
