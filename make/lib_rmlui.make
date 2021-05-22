MKFILE      := $(lastword $(MAKEFILE_LIST)) 

include make/configure.make
include make/platform.make

TARGET		 := libRmlCore

CPPSOURCES := $(MOUNT_DIR)/RmlUI/Source \
              $(MOUNT_DIR)/RmlUI/Source/Core \
						  $(MOUNT_DIR)/RmlUI/Source/Core/Elements \
							$(MOUNT_DIR)/RmlUI/Source/Core/FontEngineDefault
INCLUDES   := /Library/Frameworks/Mono.framework/Headers \
						  /usr/local/include/freetype2

#SYSROOT    := $(shell xcrun --show-sdk-path)
SYSROOT    := /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk

LIBS = /usr/local/lib/libfreetype.dylib \
         $(FREETYPE_LIBS)

CPPFILES   := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
OBJS       := $(CPPFILES:.cpp=.o)
Q3OBJ      := $(addprefix $(B)/rmlui/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CXX      = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
# -fsigned-char \
             -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
CFLAGS   = $(INCLUDE) \
						 -DRMLUI_NO_THIRDPARTY_CONTAINERS \
						 -DBUILD_FREETYPE $(FREETYPE_CFLAGS) \
						 -DRmlCore_EXPORTS \
						 -isysroot $(SYSROOT) \
						 -MMD \
						 -DRMLUI_NO_THIRDPARTY_CONTAINERS
CXXFLAGS  = $(CFLAGS)  -std=c++14

define DO_RMLUI_CXX
	$(echo_cmd) "RMLUI_CC $<"
	$(Q)$(CXX) $(SHLIBCFLAGS) $(CXXFLAGS) -o $@ -c $<
endef

$(B)/rmlui/%.o: code/RmlUi/Source/%.cpp
	$(DO_RMLUI_CXX)

$(B)/rmlui/%.o: code/RmlUi/Source/Core/%.cpp
	$(DO_RMLUI_CXX)

$(B)/rmlui/%.o: code/RmlUi/Source/Core/Elements/%.cpp
	$(DO_RMLUI_CXX)

$(B)/rmlui/%.o: code/RmlUi/Source/Core/FontEngineDefault/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@

default:
	$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=rmlui mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/rmlui
	@rm -rf $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/rmlui
	@rm -rf $(BR)/$(TARGET)$(SHLIBNAME)
