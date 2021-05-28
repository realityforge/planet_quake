MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := rmlui
RMLUIDIR      := libs/RmlUI/Source

BUILD_RMLUI   := 1
include make/platform.make

TARGET		    := libRmlCore_$(SHLIBNAME)
CPPSOURCES    := $(RMLUIDIR) \
                 $(RMLUIDIR)/Core \
                 $(RMLUIDIR)/Core/Elements \
                 $(RMLUIDIR)/Core/FontEngineDefault
INCLUDES      := 
LIBS          := $(FREETYPE_LIBS)

CPPFILES      := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
OBJS          := $(CPPFILES:.cpp=.o)
Q3OBJ         := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS        := $(INCLUDE) \
                 -DRMLUI_NO_THIRDPARTY_CONTAINERS \
                 -DBUILD_FREETYPE $(FREETYPE_CFLAGS) \
                 -DRmlCore_EXPORTS \
                 -isysroot $(SYSROOT) -MMD \
                 -DRMLUI_NO_THIRDPARTY_CONTAINERS
CXXFLAGS      := $(CFLAGS) -std=c++14

define DO_RMLUI_CXX
  $(echo_cmd) "RMLUI_CXX $<"
  $(Q)$(CXX) $(SHLIBCFLAGS) $(CXXFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/Core/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/Core/Elements/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/Core/FontEngineDefault/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS)
endif
