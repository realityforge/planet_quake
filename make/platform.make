ABSOLUTE_PATH       := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))..
ifndef COMPILE_PLATFORM
COMPILE_PLATFORM    := $(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
endif
ifndef COMPILE_ARCH
COMPILE_ARCH        := $(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')
endif

ifeq ($(COMPILE_PLATFORM),mingw32)
ifeq ($(COMPILE_ARCH),i386)
COMPILE_ARCH=x86
endif
endif

# echo_cmd is silent in verbose mode, makes sense
ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

ifeq ($(COMPILE_PLATFORM),cygwin)
PLATFORM ?= mingw32
COMPILE_PLATFORM := mingw32
endif

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(PLATFORM),mingw32)
MINGW=1
endif
ifeq ($(PLATFORM),mingw64)
MINGW=1
endif

ifeq ($(COMPILE_ARCH),i86pc)
COMPILE_ARCH=x86
endif

ifeq ($(COMPILE_ARCH),amd64)
COMPILE_ARCH=x86_64
endif
ifeq ($(COMPILE_ARCH),x64)
COMPILE_ARCH=x86_64
endif

ifndef ARCH
ARCH=$(COMPILE_ARCH)
endif
ifeq ($(PLATFORM),js)
ARCH=js
endif
ifeq ($(PLATFORM),wasm)
ARCH=js
endif
export ARCH

####################################################################

INSTALL   ?= install
MKDIR     ?= mkdir -p
MOUNT_DIR ?= code
BUILD_DIR ?= build
MAKE      ?= make

INSTALL_DIR := bin

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)
SHLIBNAME    = $(ARCH).$(SHLIBEXT)

####################################################################

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
CROSS_COMPILING=1
else
CROSS_COMPILING=0

ifneq ($(ARCH),$(COMPILE_ARCH))
CROSS_COMPILING=1
endif
endif

export CROSS_COMPILING

ifdef MINGW
include make/platform_win.make
else
ifeq ($(PLATFORM),darwin)
include make/platform_macos.make
else
ifeq ($(PLATFORM),js)
include make/platform_emjs.make
else
ifeq ($(PLATFORM),wasm)
include make/platform_wasm.make
else
COMPILE_PLATFORM    := linux
include make/platform_unix.make
endif
endif
endif
endif

D_FILES :=

ifdef WORKDIR
D_DIRS  := $(addprefix $(BD)/,$(notdir $(WORKDIRS))) \
           $(addprefix $(BR)/,$(notdir $(WORKDIRS)))
D_FILES := $(shell find $(BD)/$(WORKDIR) -name '*.d' 2>/dev/null) \
           $(shell find $(BR)/$(WORKDIR) -name '*.d' 2>/dev/null) \
           $(shell find $(D_DIRS) -name '*.d' 2>/dev/null)
ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif
endif


# make sure clean happens before build directory creation
$(BUILD_DIR).mkdirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi

$(BUILD_DIR)/%.mkdirs: $(filter $(MAKECMDGOALS),clean) 
	@if [ ! -d "./$(subst .mkdirs,,$@)/$$dir" ]; \
		then $(MKDIR) "./$(subst .mkdirs,,$@)/$$dir";fi;


.PHONY: all clean clean2 clean-debug clean-release copyfiles \
  debug default dist distclean makedirs release \
  targets tools toolsclean mkdirs build install  \
    $(D_FILES)

.NOTPARALLEL: index clean install
