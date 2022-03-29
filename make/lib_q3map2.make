MKFILE           := $(lastword $(MAKEFILE_LIST)) 
Q3MAP2_WORKDIR   := q3map2
Q3MAP2DIR        := libs/tools/quake3
Q3MAP_VERSION    := 2.5.17n
RADIANT_VERSION  := 1.5.0n
RADIANT_MAJOR_VERSION:=5
RADIANT_MINOR_VERSION:=0
LINKABLE         ?= 1

BUILD_Q3MAP2     := 1
include make/platform.make

ifeq ($(LINKABLE),1)
Q3MAP2_TARGET    := $(CNAME)_q3map2_$(SHLIBNAME)
else
Q3MAP2_TARGET    := $(CNAME)_q3map2
endif
CPPSOURCES       := $(Q3MAP2DIR)/common $(Q3MAP2DIR)/q3map2
INCLUDES         := $(Q3MAP2DIR)/common $(Q3MAP2DIR)/../libs \
										$(Q3MAP2DIR)/../include libs/libpng-1.6.37
Q3MAP2_LIBS      := $(ZLIB_LIBS) $(XML_LIBS) $(GLIB_LIBS) \
										$(STD_LIBS) $(JPEG_LIBS) $(PNG_LIBS)

Q3MAP2           := common/cmdlib.o \
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
										q3map2/main.o \
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
ifeq ($(PLATFORM),mingw32)
Q3MAP2           += l_net/l_net_wins.o
else
Q3MAP2           += l_net/l_net_berkley.o
endif

PICOMODEL        :=	lwo/clip.o \
	                  lwo/envelope.o \
	                  lwo/list.o \
	                  lwo/lwio.o \
	                  lwo/lwo2.o \
	                  lwo/lwob.o \
	                  lwo/pntspols.o \
	                  lwo/surface.o \
	                  lwo/vecmath.o \
	                  lwo/vmap.o \
	                  picointernal.o \
	                  picomodel.o \
	                  picomodules.o \
	                  pm_3ds.o \
	                  pm_ase.o \
	                  pm_fm.o \
	                  pm_lwo.o \
	                  pm_md2.o \
	                  pm_md3.o \
	                  pm_mdc.o \
	                  pm_ms3d.o \
	                  pm_obj.o \
	                  pm_terrain.o

#CPPFILES         := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
Q3MAP2_OBJ       := $(addprefix $(B)/$(Q3MAP2_WORKDIR)/,$(notdir $(Q3MAP2))) \
								    $(addprefix $(B)/$(Q3MAP2_WORKDIR)/picomodel/,$(notdir $(PICOMODEL)))

export INCLUDE	 := $(foreach dir,$(INCLUDES),-I$(dir))

Q3MAP2_CFLAGS    := $(INCLUDE) \
										$(XML_CFLAGS) $(JPEG_CFLAGS) \
										$(GLIB_CFLAGS) $(PNG_CFLAGS) -MMD \
										-DPOSIX \
										-DRADIANT_VERSION="\"$(RADIANT_VERSION)\"" \
										-DQ3MAP_VERSION="\"$(Q3MAP_VERSION)\"" \
										-DRADIANT_MAJOR_VERSION="\"$(RADIANT_MAJOR_VERSION)\"" \
										-DRADIANT_MINOR_VERSION="\"$(RADIANT_MINOR_VERSION)\""
ifdef MINGW
Q3MAP2_CFLAGS    += -D_WIN32=1 -DWIN32=1
endif

ifeq ($(LINKABLE),1)
Q3MAP2_CFLAGS    += -DLINKABLE=1
endif
Q3MAP2_CXXFLAGS  := $(Q3MAP2_CFLAGS) -std=c++17

define DO_Q3MAP2_GXX
  $(echo_cmd) "Q3MAP2_GXX $<"
  $(Q)$(GXX) $(SHLIBCFLAGS) $(Q3MAP2_CXXFLAGS) -o $@ -c $<
endef

define DO_Q3MAP2_CC
  $(echo_cmd) "Q3MAP2_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(Q3MAP2_CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(Q3MAP2_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) \
		WORKDIRS="$(Q3MAP2_WORKDIR) $(Q3MAP2_WORKDIR)/picomodel" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) -j 8 \
		CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(Q3MAP2_TARGET)

release:
	$(echo_cmd) "MAKE $(Q3MAP2_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) \
		WORKDIRS="$(Q3MAP2_WORKDIR) $(Q3MAP2_WORKDIR)/picomodel" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) -j 8 \
		CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(Q3MAP2_TARGET)

clean:
	@rm -rf ./$(BD)/$(Q3MAP2_WORKDIR)
	@rm -rf ./$(BD)/$(Q3MAP2_TARGET)
	@rm -rf ./$(BR)/$(Q3MAP2_WORKDIR)
	@rm -rf ./$(BR)/$(Q3MAP2_TARGET)

ifdef B
$(B)/$(Q3MAP2_WORKDIR)/%.o: $(Q3MAP2DIR)/common/%.cpp
	$(DO_Q3MAP2_GXX)

$(B)/$(Q3MAP2_WORKDIR)/%.o: $(Q3MAP2DIR)/q3map2/%.cpp
	$(DO_Q3MAP2_GXX)

$(B)/$(Q3MAP2_WORKDIR)/%.o: $(Q3MAP2DIR)/../libs/ddslib/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(Q3MAP2_WORKDIR)/%.o: $(Q3MAP2DIR)/../libs/l_net/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(Q3MAP2_WORKDIR)/%.o: $(Q3MAP2DIR)/../libs/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(Q3MAP2_WORKDIR)/picomodel/%.o: $(Q3MAP2DIR)/../libs/picomodel/%.c
	$(DO_Q3MAP2_CC)

$(B)/$(Q3MAP2_WORKDIR)/picomodel/%.o: $(Q3MAP2DIR)/../libs/picomodel/lwo/%.c
	$(DO_Q3MAP2_CC)

ifeq ($(LINKABLE),1)
$(B)/$(Q3MAP2_TARGET): $(Q3MAP2_OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3MAP2_OBJ) $(Q3MAP2_LIBS) $(SHLIBLDFLAGS)
else
$(B)/$(Q3MAP2_TARGET): $(Q3MAP2_OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3MAP2_OBJ) $(Q3MAP2_LIBS) $(LDFLAGS)
endif
endif
