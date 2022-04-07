ifndef MOD
MOD             := multigame
endif
WORKDIR   = $(MOD)
WORKDIRS += $(MOD) $(MOD)/cgame $(MOD)/game $(MOD)/q3_ui $(MOD)/ui $(MOD)/vm
NO_MAKE_LOCAL   := 1

#USE_CLASSIC_MENU = 1
#MISSIONPACK       = 1

BUILD_GAME_QVM  ?= 1
BUILD_BASEQ3A   := 1
BUILD_MULTIGAME := 1
ifneq ($(BUILD_CLIENT),1)
BUILD_GAME_STATIC = 0
WORKDIR         := $(MOD)
MKFILE          := $(lastword $(MAKEFILE_LIST)) 
include make/platform.make
endif

MOD_CFLAGS    :=
ifeq ($(MISSIONPACK),1)
MOD_CFLAGS    += -DMISSIONPACK
endif
ifeq ($(USE_CLASSIC_MENU),1)
MOD_CFLAGS    += -DUSE_CLASSIC_MENU
endif
ifeq ($(USE_REFEREE_CMDS),1)
MOD_CFLAGS    += -DUSE_REFEREE_CMDS
endif
ifeq ($(USE_GAME_FREEZETAG),1)
MOD_CFLAGS    += -DUSE_GAME_FREEZETAG
endif

ifndef Q3LCC
Q3ASM          = $(BR)/tools/q3asm
Q3LCC          = $(BR)/tools/q3lcc
endif


GAMEDIR       := $(MOUNT_DIR)/../games/$(MOD)/code
QADIR         := $(GAMEDIR)/game
CGDIR         := $(GAMEDIR)/cgame
UIDIR         := $(GAMEDIR)/q3_ui
SUIDIR        := $(GAMEDIR)/ui

GAME_CFLAGS   := $(BASE_CFLAGS) $(MOD_CFLAGS)
ifeq ($(CC), $(findstring $(CC), "clang" "clang++"))
GAME_CFLAGS   += -Wno-unused-arguments
endif

GAME_CFLAGS   += -Wformat=2 -Wno-format-zero-length -Wformat-security -Wno-format-nonliteral
GAME_CFLAGS   += -Wstrict-aliasing=2 -Wmissing-format-attribute
GAME_CFLAGS   += -Wdisabled-optimization -MMD
GAME_CFLAGS   += -Werror-implicit-function-declaration

ifneq ($(BUILD_CLIENT),1)
GAME_CFLAGS   += $(SHLIBCFLAGS)
GAME_LDFLAGS  += $(SHLIBLDFLAGS)
else
export GAME_INCLUDE := -I$(GAMEDIR)
endif

define DO_GAME_CC
	$(echo_cmd) "GAME_CC $<"
	$(Q)$(CC) -DQAGAME $(SHLIBCFLAGS) $(GAME_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_CGAME_CC
	$(echo_cmd) "CGAME_CC $<"
	$(Q)$(CC) -DCGAME $(SHLIBCFLAGS) $(GAME_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_UI_CC
	$(echo_cmd) "UI_CC $<"
	$(Q)$(CC) -DUI $(SHLIBCFLAGS) $(GAME_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_GAME_LCC
	$(echo_cmd) "GAME_LCC $<"
	$(Q)$(Q3LCC) -DQAGAME $(MOD_CFLAGS) -o $@ -c $<
endef

define DO_CGAME_LCC
	$(echo_cmd) "CGAME_LCC $<"
	$(Q)$(Q3LCC) -DCGAME $(MOD_CFLAGS) -o $@ -c $<
endef

define DO_UI_LCC
	$(echo_cmd) "UI_LCC $<"
	$(Q)$(Q3LCC) -DUI $(MOD_CFLAGS) -o $@ -c $<
endef

#############################################################################
# MAIN TARGETS
#############################################################################

ifneq ($(BUILD_CLIENT),1)
GAME_TARGETS := 
ifneq ($(BUILD_GAME_LIB),0)
GAME_TARGETS += $(MOD)/cgame$(SHLIBNAME) \
                $(MOD)/qagame$(SHLIBNAME) \
                $(MOD)/ui$(SHLIBNAME)
endif
ifneq ($(BUILD_GAME_QVM),0)
GAME_TARGETS += $(MOD)/vm/cgame.qvm \
                $(MOD)/vm/qagame.qvm \
                $(MOD)/vm/ui.qvm
endif
ifeq ($(GAME_TARGETS),)
$(error BUILD_GAME_LIB and BUILD_GAME_QVM switched off)
endif

debug:
	$(echo_cmd) "MAKE $(MOD)"
	@$(MAKE) -f make/lib_q3lcc.make release 
	@$(MAKE) -f $(MKFILE) -j 8 \
		$(addprefix $(BD)/,$(GAME_TARGETS)) \
		B=$(BD) GAME_CFLAGS="$(GAME_CFLAGS)" \
		OPTIMIZE="$(DEBUG_CFLAGS)" V=$(V)

release:
	$(echo_cmd) "MAKE $(MOD)"
	@$(MAKE) -f make/lib_q3lcc.make release 
	@$(MAKE) -f $(MKFILE) -j 8 \
		$(addprefix $(BR)/,$(GAME_TARGETS)) \
		B=$(BR) GAME_CFLAGS="$(GAME_CFLAGS)" \
		OPTIMIZE="-DNDEBUG $(OPTIMIZE)" V=$(V)

endif

#############################################################################
## BASEQ3 CGAME
#############################################################################
# $(B)/$(MOD)/cgame/cg_particles.o \

CGOBJ_  = $(B)/$(MOD)/cgame/cg_main.o \
          $(B)/$(MOD)/cgame/bg_misc.o \
          $(B)/$(MOD)/cgame/bg_pmove.o \
          $(B)/$(MOD)/cgame/bg_slidemove.o \
          $(B)/$(MOD)/cgame/bg_tracemap.o \
          $(B)/$(MOD)/cgame/cg_atmospheric.o \
          $(B)/$(MOD)/cgame/cg_consolecmds.o \
          $(B)/$(MOD)/cgame/cg_draw.o \
          $(B)/$(MOD)/cgame/cg_drawtools.o \
          $(B)/$(MOD)/cgame/cg_effects.o \
          $(B)/$(MOD)/cgame/cg_ents.o \
          $(B)/$(MOD)/cgame/cg_event.o \
          $(B)/$(MOD)/cgame/cg_info.o \
          $(B)/$(MOD)/cgame/cg_localents.o \
          $(B)/$(MOD)/cgame/cg_marks.o \
          $(B)/$(MOD)/cgame/cg_players.o \
          $(B)/$(MOD)/cgame/cg_playerstate.o \
          $(B)/$(MOD)/cgame/cg_polybus.o \
          $(B)/$(MOD)/cgame/cg_predict.o \
          $(B)/$(MOD)/cgame/cg_scoreboard.o \
          $(B)/$(MOD)/cgame/cg_servercmds.o \
          $(B)/$(MOD)/cgame/cg_snapshot.o \
          $(B)/$(MOD)/cgame/cg_view.o \
          $(B)/$(MOD)/cgame/cg_weapons.o
ifeq ($(MISSIONPACK),1)
CGOBJ_ += $(B)/$(MOD)/cgame/cg_newdraw.o \
          $(B)/$(MOD)/cgame/cg_shared.o
endif

CGOBJ   = $(CGOBJ_) $(B)/$(MOD)/cgame/cg_syscalls.o
CGVMOBJ = $(addprefix $(B)/$(MOD)/cgame/,$(notdir $(CGOBJ_:%.o=%.asm)))

ifneq ($(BUILD_CLIENT),1)
CGOBJ   += $(B)/$(MOD)/cgame/q_math.o \
           $(B)/$(MOD)/cgame/q_shared.o
CGVMOBJ += $(B)/$(MOD)/cgame/q_math.asm \
           $(B)/$(MOD)/cgame/q_shared.asm \
           $(B)/$(MOD)/cgame/bg_lib.asm \
           $(GAMEDIR)/cgame/cg_syscalls.asm
endif

#############################################################################
## BASEQ3 GAME
#############################################################################

QAOBJ_  = $(B)/$(MOD)/game/g_main.o \
          $(B)/$(MOD)/game/ai_chat.o \
          $(B)/$(MOD)/game/ai_cmd.o \
          $(B)/$(MOD)/game/ai_dmnet.o \
          $(B)/$(MOD)/game/ai_dmq3.o \
          $(B)/$(MOD)/game/ai_main.o \
          $(B)/$(MOD)/game/ai_team.o \
          $(B)/$(MOD)/game/ai_vcmd.o \
          $(B)/$(MOD)/game/g_active.o \
          $(B)/$(MOD)/game/g_arenas.o \
          $(B)/$(MOD)/game/g_bot.o \
          $(B)/$(MOD)/game/g_client.o \
          $(B)/$(MOD)/game/g_cmds.o \
          $(B)/$(MOD)/game/g_combat.o \
          $(B)/$(MOD)/game/g_items.o \
          $(B)/$(MOD)/game/g_mem.o \
          $(B)/$(MOD)/game/g_misc.o \
          $(B)/$(MOD)/game/g_missile.o \
          $(B)/$(MOD)/game/g_mover.o \
          $(B)/$(MOD)/game/g_rotation.o \
          $(B)/$(MOD)/game/g_session.o \
          $(B)/$(MOD)/game/g_spawn.o \
          $(B)/$(MOD)/game/g_svcmds.o \
          $(B)/$(MOD)/game/g_target.o \
          $(B)/$(MOD)/game/g_team.o \
          $(B)/$(MOD)/game/g_trigger.o \
          $(B)/$(MOD)/game/g_utils.o \
          $(B)/$(MOD)/game/g_unlagged.o \
          $(B)/$(MOD)/game/g_weapon.o

QAOBJ   = $(QAOBJ_) $(B)/$(MOD)/game/g_syscalls.o
QAVMOBJ = $(addprefix $(B)/$(MOD)/game/,$(notdir $(QAOBJ_:%.o=%.asm))) \

ifneq ($(BUILD_CLIENT),1)
QAOBJ += $(B)/$(MOD)/game/bg_misc.o \
          $(B)/$(MOD)/game/bg_pmove.o \
          $(B)/$(MOD)/game/bg_slidemove.o \
          $(B)/$(MOD)/game/bg_tracemap.o \
          $(B)/$(MOD)/game/q_math.o \
          $(B)/$(MOD)/game/q_shared.o
QAVMOBJ += $(B)/$(MOD)/game/bg_lib.asm \
           $(B)/$(MOD)/game/bg_misc.asm \
           $(B)/$(MOD)/game/bg_pmove.asm \
           $(B)/$(MOD)/game/bg_slidemove.asm \
           $(B)/$(MOD)/game/bg_tracemap.asm \
           $(B)/$(MOD)/game/q_math.asm \
           $(B)/$(MOD)/game/q_shared.asm \
           $(GAMEDIR)/game/g_syscalls.asm
endif

#############################################################################
## BASEQ3 UI
#############################################################################
UIOBJ_  = 
ifeq ($(MISSIONPACK),1)
UIOBJ_  = $(B)/$(MOD)/ui/ui_main.o \
          $(B)/$(MOD)/ui/ui_atoms.o \
          $(B)/$(MOD)/ui/ui_gameinfo.o \
          $(B)/$(MOD)/ui/ui_players.o \
          $(B)/$(MOD)/ui/ui_shared.o
endif

ifneq ($(MISSIONPACK),1)
INCLUDE_VANILLA := 1
endif

ifeq ($(USE_CLASSIC_MENU),1)
INCLUDE_VANILLA := 1
endif

ifeq ($(INCLUDE_VANILLA),1)
UIOBJ_ += $(B)/$(MOD)/q3_ui/ui_main.o \
          $(B)/$(MOD)/q3_ui/ui_addbots.o \
          $(B)/$(MOD)/q3_ui/ui_atoms.o \
          $(B)/$(MOD)/q3_ui/ui_cdkey.o \
          $(B)/$(MOD)/q3_ui/ui_cinematics.o \
          $(B)/$(MOD)/q3_ui/ui_confirm.o \
          $(B)/$(MOD)/q3_ui/ui_connect.o \
          $(B)/$(MOD)/q3_ui/ui_controls2.o \
          $(B)/$(MOD)/q3_ui/ui_credits.o \
          $(B)/$(MOD)/q3_ui/ui_demo2.o \
          $(B)/$(MOD)/q3_ui/ui_display.o \
          $(B)/$(MOD)/q3_ui/ui_gameinfo.o \
          $(B)/$(MOD)/q3_ui/ui_ingame.o \
          $(B)/$(MOD)/q3_ui/ui_loadconfig.o \
          $(B)/$(MOD)/q3_ui/ui_menu.o \
          $(B)/$(MOD)/q3_ui/ui_mfield.o \
          $(B)/$(MOD)/q3_ui/ui_mods.o \
          $(B)/$(MOD)/q3_ui/ui_network.o \
          $(B)/$(MOD)/q3_ui/ui_options.o \
          $(B)/$(MOD)/q3_ui/ui_playermodel.o \
          $(B)/$(MOD)/q3_ui/ui_players.o \
          $(B)/$(MOD)/q3_ui/ui_playersettings.o \
          $(B)/$(MOD)/q3_ui/ui_preferences.o \
          $(B)/$(MOD)/q3_ui/ui_qmenu.o \
          $(B)/$(MOD)/q3_ui/ui_removebots.o \
          $(B)/$(MOD)/q3_ui/ui_saveconfig.o \
          $(B)/$(MOD)/q3_ui/ui_serverinfo.o \
          $(B)/$(MOD)/q3_ui/ui_servers2.o \
          $(B)/$(MOD)/q3_ui/ui_setup.o \
          $(B)/$(MOD)/q3_ui/ui_sound.o \
          $(B)/$(MOD)/q3_ui/ui_sparena.o \
          $(B)/$(MOD)/q3_ui/ui_specifyserver.o \
          $(B)/$(MOD)/q3_ui/ui_splevel.o \
          $(B)/$(MOD)/q3_ui/ui_sppostgame.o \
          $(B)/$(MOD)/q3_ui/ui_spskill.o \
          $(B)/$(MOD)/q3_ui/ui_startserver.o \
          $(B)/$(MOD)/q3_ui/ui_team.o \
          $(B)/$(MOD)/q3_ui/ui_teamorders.o \
          $(B)/$(MOD)/q3_ui/ui_video.o
endif

UIOBJ   = $(UIOBJ_)
ifeq ($(MISSIONPACK),1)
UIOBJ  += $(B)/$(MOD)/ui/ui_syscalls.o
else
UIOBJ  += $(B)/$(MOD)/q3_ui/ui_syscalls.o
endif
UIVMOBJ = $(UIOBJ_:%.o=%.asm)

ifneq ($(BUILD_CLIENT),1)
UIOBJ  += $(B)/$(MOD)/ui/bg_misc.o \
          $(B)/$(MOD)/ui/q_math.o \
          $(B)/$(MOD)/ui/q_shared.o

UIVMOBJ += $(B)/$(MOD)/ui/bg_lib.asm \
           $(B)/$(MOD)/ui/bg_misc.asm \
           $(B)/$(MOD)/ui/q_math.asm \
           $(B)/$(MOD)/ui/q_shared.asm \
           $(GAMEDIR)/ui/ui_syscalls.asm
endif

#############################################################################
## GAME MODULE RULES
#############################################################################


ifdef B
# native libs

mkdirs: $(addsuffix .mkdirs,$(addprefix $(B)/,$(WORKDIRS)))

ifneq ($(BUILD_GAME_LIB),0)

$(B)/$(MOD)/cgame$(SHLIBNAME): mkdirs $(CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) $(GAME_LDFLAGS) -o $@ $(CGOBJ)

$(B)/$(MOD)/qagame$(SHLIBNAME): mkdirs $(QAOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) $(GAME_LDFLAGS) -o $@ $(QAOBJ)

$(B)/$(MOD)/ui$(SHLIBNAME): mkdirs $(UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) $(GAME_LDFLAGS) -o $@ $(UIOBJ)

endif

# qvms

ifneq ($(BUILD_GAME_QVM),0)

$(B)/$(MOD)/vm/cgame.qvm: mkdirs $(CGVMOBJ) $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ -m $(CGVMOBJ)

$(B)/$(MOD)/vm/qagame.qvm: mkdirs $(QAVMOBJ) $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ -m $(QAVMOBJ)

$(B)/$(MOD)/vm/ui.qvm: mkdirs $(UIVMOBJ) $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ -m $(UIVMOBJ)

endif

# game codes

ifneq ($(BUILD_GAME_LIB),0)

$(B)/$(MOD)/cgame/bg_%.o: $(QADIR)/bg_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/q_%.o: $(QADIR)/q_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/ui_%.o: $(SUIDIR)/ui_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/game/%.o: $(QADIR)/%.c
	$(DO_GAME_CC)

$(B)/$(MOD)/ui/bg_%.o: $(QADIR)/bg_%.c
	$(DO_UI_CC)

$(B)/$(MOD)/ui/q_%.o: $(QADIR)/q_%.c
	$(DO_UI_CC)

ifeq ($(MISSIONPACK),1)
$(B)/$(MOD)/ui/ui_%.o: $(SUIDIR)/ui_%.c
	$(DO_UI_CC)
endif

ifeq ($(INCLUDE_VANILLA),1)
$(B)/$(MOD)/q3_ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)
endif

endif

ifneq ($(BUILD_GAME_QVM),0)

$(B)/$(MOD)/cgame/bg_%.asm: $(QADIR)/bg_%.c $(Q3LCC)
	$(DO_CGAME_LCC)

$(B)/$(MOD)/cgame/q_%.asm: $(QADIR)/q_%.c $(Q3LCC)
	$(DO_CGAME_LCC)

$(B)/$(MOD)/cgame/%.asm: $(CGDIR)/%.c $(Q3LCC)
	$(DO_CGAME_LCC)

$(B)/$(MOD)/cgame/ui_%.asm: $(SUIDIR)/ui_%.c $(Q3LCC)
	$(DO_CGAME_LCC)

$(B)/$(MOD)/game/%.asm: $(QADIR)/%.c $(Q3LCC)
	$(DO_GAME_LCC)

$(B)/$(MOD)/ui/bg_%.asm: $(QADIR)/bg_%.c $(Q3LCC)
	$(DO_UI_LCC)

$(B)/$(MOD)/ui/q_%.asm: $(QADIR)/q_%.c $(Q3LCC)
	$(DO_UI_LCC)

ifeq ($(MISSIONPACK),1)
$(B)/$(MOD)/ui/ui_%.asm: $(SUIDIR)/ui_%.c $(Q3LCC)
	$(DO_UI_LCC)
endif

ifeq ($(INCLUDE_VANILLA),1)
$(B)/$(MOD)/q3_ui/%.asm: $(UIDIR)/%.c $(Q3LCC)
	$(DO_UI_LCC)
endif

endif # build qvm

endif

#############################################################################
# MISC
#############################################################################

ifneq ($(BUILD_CLIENT),1)
clean:
	@rm -rf ./$(BD)/$(MOD)/cgame
	@rm -rf ./$(BR)/$(MOD)/cgame
	@rm -rf ./$(BD)/$(MOD)/game
	@rm -rf ./$(BR)/$(MOD)/game
	@rm -rf ./$(BD)/$(MOD)/ui
	@rm -rf ./$(BR)/$(MOD)/ui
	@rm -rf ./$(BD)/$(MOD)/vm
	@rm -rf ./$(BR)/$(MOD)/vm
	@rm -f ./$(BD)/$(MOD)/cgame$(SHLIBNAME)
	@rm -f ./$(BD)/$(MOD)/game$(SHLIBNAME)
	@rm -f ./$(BD)/$(MOD)/ui$(SHLIBNAME)
	@rm -f ./$(BR)/$(MOD)/cgame$(SHLIBNAME)
	@rm -f ./$(BR)/$(MOD)/game$(SHLIBNAME)
	@rm -f ./$(BR)/$(MOD)/ui$(SHLIBNAME)
	@rm -rf ./$(BD)/$(MOD)
	@rm -rf ./$(BR)/$(MOD)
endif

GAME_OBJ  = $(QAOBJ) $(CGOBJ) $(UIOBJ)
CLEANS 	 += $(MOD)/cgame $(MOD)/game $(MOD)/q3_ui $(MOD)/ui $(MOD)/vm $(MOD)
