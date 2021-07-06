MKFILE        := $(lastword $(MAKEFILE_LIST)) 
RMLUI_WORKDIR := rmlui
RMLUIDIR      := libs/RmlUI/Source

BUILD_RMLUI   := 1
include make/platform.make

TARGET		    := libRmlCore_$(SHLIBNAME)
CPPSOURCES    := $(RMLUIDIR) \
                 $(RMLUIDIR)/Core \
                 $(RMLUIDIR)/Core/Elements \
                 $(RMLUIDIR)/Core/FontEngineDefault
INCLUDES      := 
RMLUI_LIBS    := $(FREETYPE_LIBS)
RMLUI_FILES   := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
RMLUI_OBJS    := $(RMLUI_FILES:.cpp=.o)
RMLUI_Q3OBJ   := $(addprefix $(B)/$(RMLUI_WORKDIR)/,$(notdir $(RMLUI_OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

RMLUI_CFLAGS  := $(INCLUDE) \
                 -DRMLUI_NO_THIRDPARTY_CONTAINERS \
                 -DBUILD_FREETYPE $(FREETYPE_CFLAGS) \
                 -DRmlCore_EXPORTS \
                 -isysroot $(SYSROOT) -MMD \
                 -DRMLUI_NO_THIRDPARTY_CONTAINERS
RMLUI_CXXFLAGS := $(RMLUI_CFLAGS) -std=c++14

define DO_RMLUI_CXX
  $(echo_cmd) "RMLUI_CXX $<"
  $(Q)$(CXX) $(SHLIBCFLAGS) $(RMLUI_CXXFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) RMLUI_WORKDIRS=$(RMLUI_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(RMLUI_CFLAGS) $(DEBUG_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS=$(RMLUI_WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(RMLUI_CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(RMLUI_WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(RMLUI_WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(RMLUI_WORKDIR)/%.o: $(RMLUIDIR)/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(RMLUI_WORKDIR)/%.o: $(RMLUIDIR)/Core/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(RMLUI_WORKDIR)/%.o: $(RMLUIDIR)/Core/Elements/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(RMLUI_WORKDIR)/%.o: $(RMLUIDIR)/Core/FontEngineDefault/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(TARGET): $(RMLUI_Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) -o $@ $(RMLUI_Q3OBJ) $(RMLUI_LIBS) $(SHLIBLDFLAGS)
endif
