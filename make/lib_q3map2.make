MKFILE         := $(lastword $(MAKEFILE_LIST)) 
WORKDIR        := q3map2
Q3MAP2DIR      := libs/tools/quake3

include make/configure.make
BUILD_RMLUI    := 1
include make/platform.make

TARGET		     := libq3map2
CPPSOURCES     := $(Q3MAP2DIR)/common $(Q3MAP2DIR)/q3map2 
INCLUDES       := $(Q3MAP2DIR)/../libs
LIBS           := $(FREETYPE_LIBS)

Q3MAP2         := common/cmdlib.o \
									common/imagelib.o \
									common/inout.o \
									common/jpeg.o \
									common/md4.o \
									common/mutex.o \
									common/polylib.o \
									common/scriplib.o \
									common/threads.o \
									common/unzip.o \
									common/vfs.o \
									common/miniz.o \
									q3map2/autopk3.o \
									q3map2/brush.o \
									q3map2/bspfile_abstract.o \
									q3map2/bspfile_ibsp.o \
									q3map2/bspfile_rbsp.o \
									q3map2/bsp.o \
									q3map2/convert_ase.o \
									q3map2/convert_bsp.o \
									q3map2/convert_obj.o \
									q3map2/convert_map.o \
									q3map2/decals.o \
									q3map2/exportents.o \
									q3map2/facebsp.o \
									q3map2/fog.o \
									q3map2/help.o \
									q3map2/image.o \
									q3map2/leakfile.o \
									q3map2/light_bounce.o \
									q3map2/lightmaps_ydnar.o \
									q3map2/light.o \
									q3map2/light_trace.o \
									q3map2/light_ydnar.o \
									q3map2/main.o \
									q3map2/map.o \
									q3map2/minimap.o \
									q3map2/mesh.o \
									q3map2/model.o \
									q3map2/patch.o \
									q3map2/path_init.o \
									q3map2/portals.o \
									q3map2/prtfile.o \
									q3map2/shaders.o \
									q3map2/surface_extra.o \
									q3map2/surface_foliage.o \
									q3map2/surface_fur.o \
									q3map2/surface_meta.o \
									q3map2/surface.o \
									q3map2/tjunction.o \
									q3map2/tree.o \
									q3map2/visflow.o \
									q3map2/vis.o \
									q3map2/writebsp.o \
									ddslib/ddslib.o \
									filematch.o \
									l_net/l_net.o

CPPFILES      := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
#OBJS          := $(CPPFILES:.cpp=.o)
Q3OBJ         := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(Q3MAP2)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS        := $(INCLUDE) \
                 -DRMLUI_NO_THIRDPARTY_CONTAINERS \
                 -DBUILD_FREETYPE $(FREETYPE_CFLAGS) \
                 -DRmlCore_EXPORTS \
                 -isysroot $(SYSROOT) -MMD \
                 -DRMLUI_NO_THIRDPARTY_CONTAINERS
CXXFLAGS      := $(CFLAGS) -std=c++14

define DO_Q3MAP2_CXX
  $(echo_cmd) "Q3MAP2_CXX $<"
  $(Q)$(CXX) $(SHLIBCFLAGS) $(CXXFLAGS) -o $@ -c $<
endef

define DO_Q3MAP2_CC
  @echo "CURL_CC $<"
  @$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) $(BR)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/$(WORKDIR)
	@rm -rf $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/$(WORKDIR)
	@rm -rf $(BR)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/$(WORKDIR)/%.o: $(Q3MAP2DIR)/common/%.cpp
	$(DO_Q3MAP2_CXX)

$(B)/$(WORKDIR)/%.o: $(Q3MAP2DIR)/q3map2/%.cpp
	$(DO_Q3MAP2_CXX)

$(B)/$(WORKDIR)/%.o: $(Q3MAP2DIR)/../libs/ddslib/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(WORKDIR)/%.o: $(Q3MAP2DIR)/../libs/l_net/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(WORKDIR)/%.o: $(Q3MAP2DIR)/../libs/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS)
endif
