
#SPSERVER := $(B)/client/sv_bot.o \
	  $(B)/client/sv_ccmds.o \
	  $(B)/client/sv_client.o \
	  $(B)/client/sv_filter.o \
		$(B)/client/sv_demo.o \
		$(B)/client/sv_demo_cl.o \
	  $(B)/client/sv_demo_ext.o \
		$(B)/client/sv_demo_mv.o \
	  $(B)/client/sv_game.o \
	  $(B)/client/sv_init.o \
	  $(B)/client/sv_main.o \
	  $(B)/client/sv_net_chan.o \
	  $(B)/client/sv_snapshot.o \
	  $(B)/client/sv_world.o \
		$(B)/client/sv_bsp.o

#SPBOTS   := $(B)/client/be_aas_bspq3.o \
	  $(B)/client/be_aas_cluster.o \
	  $(B)/client/be_aas_debug.o \
	  $(B)/client/be_aas_entity.o \
	  $(B)/client/be_aas_file.o \
	  $(B)/client/be_aas_main.o \
	  $(B)/client/be_aas_move.o \
	  $(B)/client/be_aas_optimize.o \
	  $(B)/client/be_aas_reach.o \
	  $(B)/client/be_aas_route.o \
	  $(B)/client/be_aas_routealt.o \
	  $(B)/client/be_aas_sample.o \
	  $(B)/client/be_ai_char.o \
	  $(B)/client/be_ai_chat.o \
	  $(B)/client/be_ai_gen.o \
	  $(B)/client/be_ai_goal.o \
	  $(B)/client/be_ai_move.o \
	  $(B)/client/be_ai_weap.o \
	  $(B)/client/be_ai_weight.o \
	  $(B)/client/be_ea.o \
	  $(B)/client/be_interface.o \
	  $(B)/client/l_crc.o \
	  $(B)/client/l_libvar.o \
	  $(B)/client/l_log.o \
	  $(B)/client/l_memory.o \
	  $(B)/client/l_precomp.o \
	  $(B)/client/l_script.o \
	  $(B)/client/l_struct.o
