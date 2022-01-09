MKFILE           := $(lastword $(MAKEFILE_LIST)) 
WORKDIR          := mbspc
MBSPCDIR         := libs/tools/mbspc
Q3MAP_VERSION    := 2.5.17n
RADIANT_VERSION  := 1.5.0n
RADIANT_MAJOR_VERSION:=5
RADIANT_MINOR_VERSION:=0

BUILD_MBSPC      := 1
include make/platform.make

MBSPC_TARGET		 := $(CNAME)_mbspc
CSOURCES         := $(MBSPCDIR)/qcommon $(MBSPCDIR)/mbspc $(MBSPCDIR)/botlib 
INCLUDES         := $(MBSPCDIR)/../libs
LIBS             := 

MBSPC            := mbspc/aas_areamerging.o \
									  mbspc/aas_cfg.o \
									  mbspc/aas_create.o \
									  mbspc/aas_edgemelting.o \
									  mbspc/aas_facemerging.o \
									  mbspc/aas_file.o \
									  mbspc/aas_gsubdiv.o \
									  mbspc/aas_map.o \
									  mbspc/aas_prunenodes.o \
									  mbspc/aas_store.o \
									  mbspc/be_aas_bspc.o \
									  mbspc/brushbsp.o \
									  mbspc/l_bsp_ent.o \
									  mbspc/l_bsp_hl.o \
									  mbspc/l_bsp_q1.o \
									  mbspc/l_bsp_q2.o \
									  mbspc/l_bsp_q3.o \
									  mbspc/l_bsp_sin.o \
									  mbspc/bspc.o \
									  mbspc/l_cmd.o \
									  mbspc/csg.o \
									  mbspc/faces.o \
									  mbspc/glfile.o \
									  mbspc/leakfile.o \
									  mbspc/l_log.o \
									  mbspc/map.o \
									  mbspc/map_hl.o \
									  mbspc/map_q1.o \
									  mbspc/map_q2.o \
									  mbspc/map_q3.o \
									  mbspc/map_sin.o \
									  mbspc/l_math.o \
									  mbspc/l_mem.o \
									  mbspc/nodraw.o \
									  mbspc/l_poly.o \
									  mbspc/portals.o \
									  mbspc/prtfile.o \
									  mbspc/l_qfiles.o \
									  mbspc/textures.o \
									  mbspc/l_threads.o \
									  mbspc/tree.o \
									  mbspc/l_utils.o \
									  mbspc/writebsp.o \
									  \
									  botlib/be_aas_bspq3.o \
									  botlib/be_aas_cluster.o \
									  botlib/be_aas_move.o \
									  botlib/be_aas_optimize.o \
									  botlib/be_aas_reach.o \
									  botlib/be_aas_sample.o \
									  botlib/l_libvar.o \
									  botlib/l_precomp.o \
									  botlib/l_script.o \
									  botlib/l_struct.o \
									  \
									  qcommon/cm_load.o \
									  qcommon/cm_patch.o \
									  qcommon/cm_test.o \
									  qcommon/cm_trace.o \
									  qcommon/md4.o \
									  qcommon/unzip.o

CFILES           := $(foreach dir,$(CSOURCES), $(wildcard $(dir)/*.c))
MBSPC_OBJ        := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(MBSPC)))

export INCLUDE	 := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS           := -c -O3 -Wall -Wstrict-prototypes -fno-strict-aliasing \
										-I/usr/include/machine \
										-Ilibs/tools/mbspc/qcommon \
										$(INCLUDE) \
            				-MMD -DNDEBUG -DBSPC -DBSPCINCLUDE

define DO_MBSPC_CC
  $(echo_cmd) "MBSPC_CC $<"
  $(Q)$(CC) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(MBSPC_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(MBSPC_TARGET)

release:
	$(echo_cmd) "MAKE $(MBSPC_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(MBSPC_TARGET)

clean:
	@rm -rf ./$(BD)/$(WORKDIR)
	@rm -rf ./$(BD)/$(MBSPC_TARGET)
	@rm -rf ./$(BR)/$(WORKDIR)
	@rm -rf ./$(BR)/$(MBSPC_TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(MBSPCDIR)/qcommon/%.c
	$(DO_MBSPC_CC)

$(B)/$(WORKDIR)/%.o: $(MBSPCDIR)/mbspc/%.c
	$(DO_MBSPC_CC)

$(B)/$(WORKDIR)/%.o: $(MBSPCDIR)/botlib/%.c
	$(DO_MBSPC_CC)

$(B)/$(MBSPC_TARGET): $(MBSPC_OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(MBSPC_OBJ) $(LIBS) $(BSP_LDFLAGS)
endif
