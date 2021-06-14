ifndef MOD
MOD           := baseq3a
endif

BUILD_BASEQ3A := 1
ifneq ($(BUILD_CLIENT),1)
MKFILE        := $(lastword $(MAKEFILE_LIST)) 
include make/platform.make
endif

GAMEDIR       := $(MOUNT_DIR)/../games/baseq3a/code
QADIR         := $(GAMEDIR)/game
CGDIR         := $(GAMEDIR)/cgame
UIDIR         := $(GAMEDIR)/q3_ui

GAME_CFLAGS   ?= $(CFLAGS)
ifeq ($(CC), $(findstring $(CC), "clang" "clang++"))
GAME_CFLAGS   += -Qunused-arguments
endif

GAME_CFLAGS   += -Wformat=2 -Wno-format-zero-length -Wformat-security -Wno-format-nonliteral
GAME_CFLAGS   += -Wstrict-aliasing=2 -Wmissing-format-attribute
GAME_CFLAGS   += -Wdisabled-optimization -MMD
GAME_CFLAGS   += -Werror-implicit-function-declaration

ifneq ($(BUILD_CLIENT),1)
GAME_CFLAGS   += $(SHLIBCFLAGS)
endif

define DO_GAME_CC
	$(echo_cmd) "GAME_CC $<"
	$(Q)$(CC) $(MOD_CFLAGS) -DQAGAME $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_CGAME_CC
	$(echo_cmd) "CGAME_CC $<"
	$(Q)$(CC) $(MOD_CFLAGS) -DCGAME $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_UI_CC
	$(echo_cmd) "UI_CC $<"
	$(Q)$(CC) $(MOD_CFLAGS) -DUI $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

#############################################################################
# MAIN TARGETS
#############################################################################

ifneq ($(BUILD_CLIENT),1)
debug:
  @$(MAKE) -f $(MKFILE) makegamedirs \
    $(BD)/$(MOD)/cgame$(SHLIBNAME) \
    $(BD)/$(MOD)/qagame$(SHLIBNAME) \
    $(BD)/$(MOD)/ui$(SHLIBNAME)\
    B=$(BD) GAME_CFLAGS="$(GAME_CFLAGS)" \
    OPTIMIZE="$(DEBUG_CFLAGS)" V=$(V)

release:
  @$(MAKE) -f $(MKFILE) makegamedirs \
    $(BR)/$(MOD)/cgame$(SHLIBNAME) \
    $(BR)/$(MOD)/qagame$(SHLIBNAME) \
    $(BR)/$(MOD)/ui$(SHLIBNAME) \
     B=$(BR) GAME_CFLAGS="$(GAME_CFLAGS)" \
    OPTIMIZE="-DNDEBUG $(OPTIMIZE)" V=$(V)

makedirs:
  @if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
  @if [ ! -d $(B) ];then $(MKDIR) $(B);fi
  @if [ ! -d $(B)/$(MOD) ];then $(MKDIR) $(B)/$(MOD);fi
  @if [ ! -d $(B)/$(MOD)/cgame ];then $(MKDIR) $(B)/$(MOD)/cgame;fi
  @if [ ! -d $(B)/$(MOD)/game ];then $(MKDIR) $(B)/$(MOD)/game;fi
  @if [ ! -d $(B)/$(MOD)/ui ];then $(MKDIR) $(B)/$(MOD)/ui;fi
  @if [ ! -d $(B)/$(MOD)/vm ];then $(MKDIR) $(B)/$(MOD)/vm;fi
endif

#############################################################################
## BASEQ3 CGAME
#############################################################################
# $(B)/$(MOD)/cgame/cg_particles.o \

CGOBJ_  = $(B)/$(MOD)/cgame/cg_main.o \
				  $(B)/$(MOD)/cgame/bg_lib.o \
				  $(B)/$(MOD)/cgame/bg_misc.o \
				  $(B)/$(MOD)/cgame/bg_pmove.o \
				  $(B)/$(MOD)/cgame/bg_slidemove.o \
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
				  $(B)/$(MOD)/cgame/cg_predict.o \
				  $(B)/$(MOD)/cgame/cg_scoreboard.o \
				  $(B)/$(MOD)/cgame/cg_servercmds.o \
				  $(B)/$(MOD)/cgame/cg_snapshot.o \
				  $(B)/$(MOD)/cgame/cg_view.o \
				  $(B)/$(MOD)/cgame/cg_weapons.o \
				  \
				  $(B)/$(MOD)/game/q_math.o \
				  $(B)/$(MOD)/game/q_shared.o

CGOBJ   = $(CGOBJ_) $(B)/$(MOD)/cgame/cg_syscalls.o

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
				  $(B)/$(MOD)/game/bg_lib.o \
				  $(B)/$(MOD)/game/bg_misc.o \
				  $(B)/$(MOD)/game/bg_pmove.o \
				  $(B)/$(MOD)/game/bg_slidemove.o \
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
				  $(B)/$(MOD)/game/g_weapon.o \
				  \
				  $(B)/$(MOD)/game/q_math.o \
				  $(B)/$(MOD)/game/q_shared.o

QAOBJ   = $(QAOBJ_) $(B)/$(MOD)/game/g_syscalls.o

#############################################################################
## BASEQ3 UI
#############################################################################

UIOBJ_  = $(B)/$(MOD)/ui/ui_main.o \
				  $(B)/$(MOD)/ui/bg_misc.o \
				  $(B)/$(MOD)/ui/bg_lib.o \
				  $(B)/$(MOD)/ui/ui_addbots.o \
				  $(B)/$(MOD)/ui/ui_atoms.o \
				  $(B)/$(MOD)/ui/ui_cdkey.o \
				  $(B)/$(MOD)/ui/ui_cinematics.o \
				  $(B)/$(MOD)/ui/ui_confirm.o \
				  $(B)/$(MOD)/ui/ui_connect.o \
				  $(B)/$(MOD)/ui/ui_controls2.o \
				  $(B)/$(MOD)/ui/ui_credits.o \
				  $(B)/$(MOD)/ui/ui_demo2.o \
				  $(B)/$(MOD)/ui/ui_display.o \
				  $(B)/$(MOD)/ui/ui_gameinfo.o \
				  $(B)/$(MOD)/ui/ui_ingame.o \
				  $(B)/$(MOD)/ui/ui_loadconfig.o \
				  $(B)/$(MOD)/ui/ui_menu.o \
				  $(B)/$(MOD)/ui/ui_mfield.o \
				  $(B)/$(MOD)/ui/ui_mods.o \
				  $(B)/$(MOD)/ui/ui_network.o \
				  $(B)/$(MOD)/ui/ui_options.o \
				  $(B)/$(MOD)/ui/ui_playermodel.o \
				  $(B)/$(MOD)/ui/ui_players.o \
				  $(B)/$(MOD)/ui/ui_playersettings.o \
				  $(B)/$(MOD)/ui/ui_preferences.o \
				  $(B)/$(MOD)/ui/ui_qmenu.o \
				  $(B)/$(MOD)/ui/ui_removebots.o \
				  $(B)/$(MOD)/ui/ui_saveconfig.o \
				  $(B)/$(MOD)/ui/ui_serverinfo.o \
				  $(B)/$(MOD)/ui/ui_servers2.o \
				  $(B)/$(MOD)/ui/ui_setup.o \
				  $(B)/$(MOD)/ui/ui_sound.o \
				  $(B)/$(MOD)/ui/ui_sparena.o \
				  $(B)/$(MOD)/ui/ui_specifyserver.o \
				  $(B)/$(MOD)/ui/ui_splevel.o \
				  $(B)/$(MOD)/ui/ui_sppostgame.o \
				  $(B)/$(MOD)/ui/ui_spskill.o \
				  $(B)/$(MOD)/ui/ui_startserver.o \
				  $(B)/$(MOD)/ui/ui_team.o \
				  $(B)/$(MOD)/ui/ui_teamorders.o \
				  $(B)/$(MOD)/ui/ui_video.o \
				  \
				  $(B)/$(MOD)/game/q_math.o \
				  $(B)/$(MOD)/game/q_shared.o

UIOBJ   = $(UIOBJ_) $(B)/$(MOD)/ui/ui_syscalls.o

#$(B)/$(BASEGAME)/vm/cgame.qvm: $(Q3CGVMOBJ) $(CGDIR)/cg_syscalls.asm $(Q3ASM)
#	$(echo_cmd) "Q3ASM $@"
#	$(Q)$(Q3ASM) -o $@ $(Q3CGVMOBJ) $(CGDIR)/cg_syscalls.asm

#############################################################################
## GAME MODULE RULES
#############################################################################

ifdef B
$(B)/$(MOD)/cgame$(SHLIBNAME): $(CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) -o $@ $(CGOBJ)

$(B)/$(MOD)/qagame$(SHLIBNAME): $(QAOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) -o $@ $(QAOBJ)

$(B)/$(MOD)/ui$(SHLIBNAME): $(UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) -o $@ $(UIOBJ)

$(B)/$(MOD)/cgame/bg_%.o: $(QADIR)/bg_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/q_%.o: $(QADIR)/q_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/game/%.o: $(QADIR)/%.c
	$(DO_GAME_CC)

$(B)/$(MOD)/ui/bg_%.o: $(QADIR)/bg_%.c
	$(DO_UI_CC)

$(B)/$(MOD)/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)
endif

#############################################################################
# MISC
#############################################################################

GAME_OBJ = $(QAOBJ) $(CGOBJ) $(UIOBJ)

ifneq ($(BUILD_CLIENT),1)
clean: clean-debug clean-release

clean-debug:
  @$(MAKE) clean2 B=$(BD)

clean-release:
  @$(MAKE) clean2 B=$(BR)

clean2:
  @echo "CLEAN $(B)"
  @rm -f $(GAME_OBJ)
  @rm -f $(B)/$(MOD)/cgame$(SHLIBNAME)
  @rm -f $(B)/$(MOD)/qagame$(SHLIBNAME)
  @rm -f $(B)/$(MOD)/ui$(SHLIBNAME)

distclean: clean
  @rm -rf $(BUILD_DIR)
else
WORKDIRS += $(MOD) $(MOD)/cgame $(MOD)/game $(MOD)/ui $(MOD)/vm
CLEANS 	 += $(MOD)/cgame $(MOD)/qagame $(MOD)/ui $(MOD)
endif
