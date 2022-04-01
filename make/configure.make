
ifndef NO_MAKE_LOCAL
include Makefile.local
endif

include make/config-defaults.make
include make/config-libs.make

help:
	@echo Please see docs: https://github.com/briancullinan/planet_quake/blob/master/docs/make.md
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n $(subst \space, ,$(addsuffix \n,$(MAKES))) \033[36m\033[0m\n"} /^${subst (|,(,$(HELPFILTER)}[a-zA-Z0-9_-]*:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MKFILE)

.DEFAULT_GOAL := help
#.RECIPEPREFIX +=
