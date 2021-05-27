MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := rmlui
RMLUIDIR      := libs/RmlUI/Source

include make/configure.make
BUILD_RMLUI   := 1
include make/platform.make

TARGET		    := libRmlCore_
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
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)$(SHLIBNAME)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/$(WORKDIR)
	@rm -rf $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/$(WORKDIR)
	@rm -rf $(BR)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/Core/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/Core/Elements/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(WORKDIR)/%.o: $(RMLUIDIR)/Core/FontEngineDefault/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS)
endif
