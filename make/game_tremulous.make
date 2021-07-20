MKFILE        := $(lastword $(MAKEFILE_LIST)) 
ifndef MOD
MOD           := tremulous
endif

BUILD_GAME_QVM  ?= 1
BUILD_TREMULOUS := 1
include make/platform.make

GAMEDIR       := $(MOUNT_DIR)/../games/$(MOD)/code
QCOMM         := $(GAMEDIR)/qcommon
QADIR         := $(GAMEDIR)/game
CGDIR         := $(GAMEDIR)/cgame
UIDIR         := $(GAMEDIR)/ui

GAME_CFLAGS   := $(BASE_CFLAGS)
ifeq ($(CC), $(findstring $(CC), "clang" "clang++"))
GAME_CFLAGS   += -Wno-unused-arguments
endif

GAME_CFLAGS   += -Wformat=2 -Wno-format-zero-length -Wformat-security -Wno-format-nonliteral
GAME_CFLAGS   += -Wstrict-aliasing=2 -Wmissing-format-attribute
GAME_CFLAGS   += -Wdisabled-optimization -MMD
GAME_CFLAGS   += -Werror-implicit-function-declaration

GAME_CFLAGS   += $(SHLIBCFLAGS)
GAME_LDFLAGS  += $(SHLIBLDFLAGS)

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

makegamedirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/$(MOD) ];then $(MKDIR) $(B)/$(MOD);fi
	@if [ ! -d $(B)/$(MOD)/cgame ];then $(MKDIR) $(B)/$(MOD)/cgame;fi
	@if [ ! -d $(B)/$(MOD)/game ];then $(MKDIR) $(B)/$(MOD)/game;fi
	@if [ ! -d $(B)/$(MOD)/ui ];then $(MKDIR) $(B)/$(MOD)/ui;fi
	@if [ ! -d $(B)/$(MOD)/vm ];then $(MKDIR) $(B)/$(MOD)/vm;fi

#############################################################################
## TREMULOUS CGAME
#############################################################################
# $(B)/$(MOD)/cgame/cg_particles.o \

CGOBJ_  = $(B)/$(MOD)/cgame/cg_animation.o \
          $(B)/$(MOD)/cgame/cg_animmapobj.o \
          $(B)/$(MOD)/cgame/cg_attachment.o \
          $(B)/$(MOD)/cgame/cg_buildable.o \
          $(B)/$(MOD)/cgame/cg_main.o \
          $(B)/$(MOD)/cgame/bg_lib.o \
          $(B)/$(MOD)/cgame/bg_misc.o \
          $(B)/$(MOD)/cgame/bg_pmove.o \
          $(B)/$(MOD)/cgame/bg_slidemove.o \
          $(B)/$(MOD)/cgame/cg_consolecmds.o \
          $(B)/$(MOD)/cgame/cg_draw.o \
          $(B)/$(MOD)/cgame/cg_drawtools.o \
          $(B)/$(MOD)/cgame/cg_ents.o \
          $(B)/$(MOD)/cgame/cg_event.o \
          $(B)/$(MOD)/cgame/cg_marks.o \
					$(B)/$(MOD)/cgame/cg_particles.o \
          $(B)/$(MOD)/cgame/cg_players.o \
          $(B)/$(MOD)/cgame/cg_playerstate.o \
          $(B)/$(MOD)/cgame/cg_predict.o \
          $(B)/$(MOD)/cgame/cg_scanner.o \
          $(B)/$(MOD)/cgame/cg_servercmds.o \
          $(B)/$(MOD)/cgame/cg_snapshot.o \
          $(B)/$(MOD)/cgame/cg_trails.o \
          $(B)/$(MOD)/cgame/cg_tutorial.o \
          $(B)/$(MOD)/cgame/cg_view.o \
          $(B)/$(MOD)/cgame/cg_weapons.o

CGOBJ_ += $(B)/$(MOD)/cgame/q_math.o \
          $(B)/$(MOD)/cgame/q_shared.o

CGOBJ   = $(CGOBJ_) $(B)/$(MOD)/cgame/cg_syscalls.o

#############################################################################
## TREMULOUS GAME
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

QAOBJ_ += $(B)/$(MOD)/game/bg_lib.o \
          $(B)/$(MOD)/game/bg_misc.o \
          $(B)/$(MOD)/game/bg_pmove.o \
          $(B)/$(MOD)/game/bg_slidemove.o \
          $(B)/$(MOD)/game/q_math.o \
          $(B)/$(MOD)/game/q_shared.o

QAOBJ   = $(QAOBJ_) $(B)/$(MOD)/game/g_syscalls.o

#############################################################################
## TREMULOUS UI
#############################################################################

UIOBJ_  = $(B)/$(MOD)/ui/ui_main.o \
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
          $(B)/$(MOD)/ui/ui_video.o

UIOBJ_ += $(B)/$(MOD)/ui/bg_misc.o \
          $(B)/$(MOD)/ui/bg_lib.o \
          $(B)/$(MOD)/ui/q_math.o \
          $(B)/$(MOD)/ui/q_shared.o

UIOBJ   = $(UIOBJ_) $(B)/$(MOD)/ui/ui_syscalls.o

#############################################################################
## GAME MODULE RULES
#############################################################################

ifdef B

# dlls

$(B)/$(MOD)/cgame$(SHLIBNAME): $(CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) $(GAME_LDFLAGS) -o $@ $(CGOBJ)

$(B)/$(MOD)/qagame$(SHLIBNAME): $(QAOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) $(GAME_LDFLAGS) -o $@ $(QAOBJ)

$(B)/$(MOD)/ui$(SHLIBNAME): $(UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(GAME_CFLAGS) $(GAME_LDFLAGS) -o $@ $(UIOBJ)

# c files

$(B)/$(MOD)/cgame/%.o: $(QCOMM)/%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/bg_%.o: $(QADIR)/bg_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/q_%.o: $(QADIR)/q_%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/game/%.o: $(QCOMM)/%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/game/%.o: $(QADIR)/%.c
	$(DO_GAME_CC)
	
$(B)/$(MOD)/ui/%.o: $(QCOMM)/%.c
	$(DO_CGAME_CC)

$(B)/$(MOD)/ui/bg_%.o: $(QADIR)/bg_%.c
	$(DO_UI_CC)

$(B)/$(MOD)/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)
endif

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

clean2:
	@echo "CLEAN $(B)"
	@rm -rf ./$(BD)/$(MOD)/cgame
	@rm -rf ./$(BR)/$(MOD)/cgame
	@rm -rf ./$(BD)/$(MOD)/game
	@rm -rf ./$(BR)/$(MOD)/game
	@rm -rf ./$(BD)/$(MOD)/ui
	@rm -rf ./$(BR)/$(MOD)/ui
	@rm -rf ./$(BD)/$(MOD)/vm
	@rm -rf ./$(BR)/$(MOD)/vm
	@rm -f ./$(B)/$(MOD)/cgame$(SHLIBNAME)
	@rm -f ./$(B)/$(MOD)/qagame$(SHLIBNAME)
	@rm -f ./$(B)/$(MOD)/ui$(SHLIBNAME)
