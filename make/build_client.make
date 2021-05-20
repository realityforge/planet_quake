MKFILE      := $(lastword $(MAKEFILE_LIST)) 

include make/platform.make
include make/configure.make
include make/platform_os.make

RENDERER_PREFIX  := $(CNAME)
TARGET	         := $(CNAME)

SOURCES  := $(MOUNT_DIR)/client

CLIPMAP  := $(B)/client/cm_load.o \
						$(B)/client/cm_load_bsp2.o \
						$(B)/client/cm_patch.o \
						$(B)/client/cm_polylib.o \
						$(B)/client/cm_test.o \
						$(B)/client/cm_trace.o

QCOMMON  := $(B)/client/cmd.o \
					  $(B)/client/common.o \
					  $(B)/client/cvar.o \
					  $(B)/client/files.o \
					  $(B)/client/history.o \
					  $(B)/client/keys.o \
					  $(B)/client/md4.o \
					  $(B)/client/md5.o \
					  $(B)/client/msg.o \
					  $(B)/client/net_chan.o \
					  $(B)/client/net_ip.o \
						$(B)/client/qrcodegen.o \
					  $(B)/client/huffman.o \
					  $(B)/client/huffman_static.o \
						$(B)/client/q_math.o \
					  $(B)/client/q_shared.o \
					  $(B)/client/unzip.o \
					  $(B)/client/puff.o \
						$(B)/client/sv_init.o \
						$(B)/client/sv_main.o \
						$(B)/client/sv_bot.o \
						$(B)/client/sv_game.o

SOUND    := $(B)/client/snd_adpcm.o \
					  $(B)/client/snd_dma.o \
					  $(B)/client/snd_mem.o \
					  $(B)/client/snd_mix.o \
					  $(B)/client/snd_wavelet.o \
					  \
					  $(B)/client/snd_main.o \
					  $(B)/client/snd_codec.o \
					  $(B)/client/snd_codec_wav.o \
						$(B)/client/snd_codec_ogg.o \
						$(B)/client/snd_codec_opus.o
ifeq ($(ARCH),x86)
ifndef MINGW
  SOUND   += \
					  $(B)/client/snd_mix_mmx.o \
					  $(B)/client/snd_mix_sse.o
endif
endif
	  
VM   := \
					$(B)/client/vm.o \
					$(B)/client/vm_interpreted.o
ifeq ($(HAVE_VM_COMPILED),true)
ifeq ($(ARCH),x86)
  VM += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),x86_64)
  VM += $(B)/client/vm_x86.o
endif
ifeq ($(ARCH),arm)
  VM += $(B)/client/vm_armv7l.o
endif
ifeq ($(ARCH),aarch64)
  VM += $(B)/client/vm_aarch64.o
endif
endif

CURL   :=
ifeq ($(USE_CURL),1)
  CURL += $(B)/client/cl_curl.o
endif


Q3OBJ   :=
ifdef MINGW
  Q3OBJ += \
    $(B)/client/win_main.o \
    $(B)/client/win_shared.o \
    $(B)/client/win_syscon.o \
    $(B)/client/win_resource.o

ifeq ($(USE_SDL),1)
ifneq ($(PLATFORM),js)
  Q3OBJ += \
    $(B)/client/sdl_glimp.o \
    $(B)/client/sdl_gamma.o \
    $(B)/client/sdl_input.o \
    $(B)/client/sdl_snd.o
endif

else # !USE_SDL
  Q3OBJ += \
    $(B)/client/win_gamma.o \
    $(B)/client/win_glimp.o \
    $(B)/client/win_input.o \
    $(B)/client/win_minimize.o \
    $(B)/client/win_qgl.o \
    $(B)/client/win_snd.o \
    $(B)/client/win_wndproc.o
ifeq ($(USE_VULKAN_API),1)
  Q3OBJ += \
      $(B)/client/win_qvk.o
endif
endif # !USE_SDL

else # !MINGW
ifeq ($(PLATFORM),js)
  Q3OBJ += \
		$(B)/client/sdl_glimp.o \
		$(B)/client/sdl_gamma.o \
		$(B)/client/sdl_snd.o \
		$(B)/client/sys_main.o \
		$(B)/client/sys_input.o

else
  Q3OBJ += \
    $(B)/client/unix_main.o \
    $(B)/client/unix_shared.o \
    $(B)/client/linux_signals.o
endif

ifeq ($(USE_SDL),1)
ifneq ($(PLATFORM),js)
  Q3OBJ += \
    $(B)/client/sdl_glimp.o \
    $(B)/client/sdl_gamma.o \
    $(B)/client/sdl_input.o \
    $(B)/client/sdl_snd.o
endif
else # !USE_SDL
  Q3OBJ += \
    $(B)/client/linux_glimp.o \
    $(B)/client/linux_qgl.o \
    $(B)/client/linux_snd.o \
    $(B)/client/x11_dga.o \
    $(B)/client/x11_randr.o \
    $(B)/client/x11_vidmode.o

ifeq ($(USE_VULKAN_API),1)
  Q3OBJ += \
      $(B)/client/linux_qvk.o
endif
endif # !USE_SDL
endif


INCLUDES := 

#LIBS = -l
CFILES         := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
									$(CLIPMAP) \
									$(QCOMMON) \
									$(SOUND) \
									$(VM) \
									$(CURL) \
									$(Q3OBJ)
OBJS          := $(CFILES:.c=.o) 
Q3OBJ         := $(addprefix $(B)/client/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

PREFIX   := 
CC       := gcc
CFLAGS   := $(INCLUDE) -fsigned-char \
             -O2 -ftree-vectorize -g -ffast-math -fno-short-enums \
						 -MMD -DBUILD_SLIM_CLIENT \
						 -DUSE_RENDERER_DLOPEN \
						 -DRENDERER_PREFIX=\"$(RENDERER_PREFIX)\"
#						 -DUSE_SYSTEM_JPEG
#LDFLAGS  := -L$(MOUNT_DIR)/macosx -lxml2 -lpng \
						$(MOUNT_DIR)/macosx/libxml2.2.dylib $(MOUNT_DIR)/macosx/libpng.dylib \
						-L$(MOUNT_DIR)/macosx -I$(MOUNT_DIR)/RmlUi/Include
LDFLAGS  := -L$(BD) -ljpeg \
						$(BD)/quake3e_libbots_x86_64.dylib

# TODO build quake 3 as a library that can be use for rendering embedded in other apps?
#SHLIBCFLAGS  = -fPIC -fno-common \
							 -DUSE_RENDERER_DLOPEN \
							 -DRENDERER_PREFIX=\"$(RENDERER_PREFIX)\"
#SHLIBLDFLAGS = -dynamiclib $(LDFLAGS)
#SHLIBNAME    = $(ARCH).$(SHLIBEXT)

define DO_CLIENT_CC
	$(echo_cmd) "CLIENT_CC $<"
	$(Q)$(CC) $(CFLAGS) $(SDL_INCLUDE) -o $@ -c $<
endef

define DO_AS
$(echo_cmd) "AS $<"
$(Q)$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
endef

define DO_TOOLS
$(echo_cmd) "TOOLS_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -I$(TDIR)/libs -I$(TDIR)/include -I$(TDIR)/common -o $@ -c $<
endef

define DO_WINDRES
$(echo_cmd) "WINDRES $<"
$(Q)$(WINDRES) -i $< -o $@
endef

mkdirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/client ];then $(MKDIR) $(B)/client;fi
# TODO: make all these dylibs
#	@if [ ! -d $(B)/client/asm ];then $(MKDIR) $(B)/client/asm;fi
#	@if [ ! -d $(B)/client/ogg ];then $(MKDIR) $(B)/client/ogg;fi
#	@if [ ! -d $(B)/client/vorbis ];then $(MKDIR) $(B)/client/vorbis;fi
#	@if [ ! -d $(B)/client/opus ];then $(MKDIR) $(B)/client/opus;fi
#	@if [ ! -d $(B)/client/q3map2 ];then $(MKDIR) $(B)/client/q3map2;fi
#	@if [ ! -d $(B)/client/tools ];then $(MKDIR) $(B)/client/tools;fi
#	@if [ ! -d $(B)/client/libs ];then $(MKDIR) $(B)/client/libs;fi

default:
	$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)

#debug:
#	@$(MAKE) -f $(MKFILE) $(TARGETS) B=$(BD) CFLAGS="$(CFLAGS) $(BASE_CFLAGS)" \
#	  OPTIMIZE="$(DEBUG_CFLAGS)" V=$(V)

#release:
#	@$(MAKE) -f $(MKFILE) $(TARGETS) B=$(BR) CFLAGS="$(CFLAGS) $(BASE_CFLAGS)" \
#	  OPTIMIZE="-DNDEBUG $(OPTIMIZE)" V=$(V)


$(B)/client/%.o: code/client/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/unix/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/win32/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/macosx/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/sdl/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/qcommon/%.c
	$(DO_CLIENT_CC)

$(B)/client/%.o: code/server/%.c
	$(DO_CLIENT_CC)

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

# Q3MAP2 += \
		$(B)/client/bg_misc.o \
	  $(B)/client/q3map2/bsp.o \
		$(B)/client/tools/inout.o \
		$(B)/client/q3map2/portals.o \
		$(B)/client/q3map2/surface.o \
		$(B)/client/q3map2/surface_meta.o \
		$(B)/client/q3map2/surface_foliage.o \
		$(B)/client/q3map2/facebsp.o \
		$(B)/client/q3map2/brush.o \
		$(B)/client/q3map2/map.o \
		$(B)/client/tools/polylib.o \
		$(B)/client/q3map2/fog.o \
		$(B)/client/q3map2/writebsp.o \
		$(B)/client/q3map2/model.o \
		$(B)/client/q3map2/shaders.o \
		$(B)/client/libs/mathlib.o \
		$(B)/client/q3map2/brush_primit.o \
		$(B)/client/q3map2/mesh.o \
		$(B)/client/q3map2/tjunction.o \
		$(B)/client/q3map2/tree.o \
		$(B)/client/q3map2/image.o \
		$(B)/client/q3map2/light.o \
		$(B)/client/q3map2/light_ydnar.o \
		$(B)/client/q3map2/light_trace.o \
		$(B)/client/q3map2/lightmaps_ydnar.o \
		$(B)/client/tools/jpeg.o \
		$(B)/client/libs/ddslib.o \
		$(B)/client/q3map2/leakfile.o \
		$(B)/client/tools/imagelib.o \
		$(B)/client/q3map2/decals.o \
		$(B)/client/q3map2/patch.o \
		$(B)/client/libs/picomodel.o \
		$(B)/client/libs/picointernal.o \
		$(B)/client/libs/picomodules.o \
		$(B)/client/q3map2/light_bounce.o \
		$(B)/client/tools/threads.o \
		$(B)/client/q3map2/surface_extra.o \
		$(B)/client/libs/m4x4.o \
		$(B)/client/libs/md5lib.o \
		$(B)/client/libs/pm_terrain.o \
		$(B)/client/libs/pm_md3.o \
		$(B)/client/libs/pm_ase.o \
		$(B)/client/libs/pm_3ds.o \
		$(B)/client/libs/pm_md2.o \
		$(B)/client/libs/pm_fm.o \
		$(B)/client/libs/pm_lwo.o \
		$(B)/client/libs/pm_mdc.o \
		$(B)/client/libs/pm_ms3d.o \
		$(B)/client/libs/pm_obj.o \
		$(B)/client/tools/vfs.o \
		$(B)/client/libs/lwo2.o \
		$(B)/client/libs/pntspols.o \
		$(B)/client/libs/vmap.o \
		$(B)/client/libs/lwob.o \
		$(B)/client/libs/clip.o \
		$(B)/client/libs/lwio.o \
		$(B)/client/libs/surface.o \
		$(B)/client/libs/list.o \
		$(B)/client/libs/envelope.o \
		$(B)/client/q3map2/surface_fur.o \
		$(B)/client/libs/vecmath.o \
		$(B)/client/tools/scriplib.o \
		$(B)/client/q3map2/prtfile.o \
	  $(B)/client/q3map2/bspfile_abstract.o \
		$(B)/client/q3map2/bspfile_rbsp.o \
		$(B)/client/q3map2/bspfile_ibsp.o

# VORBIS += \
	  $(B)/client/ogg/bitwise.o \
	  $(B)/client/ogg/framing.o \
	  \
	  $(B)/client/vorbis/analysis.o \
	  $(B)/client/vorbis/barkmel.o \
	  $(B)/client/vorbis/bitrate.o \
	  $(B)/client/vorbis/block.o \
	  $(B)/client/vorbis/codebook.o \
	  $(B)/client/vorbis/floor0.o \
	  $(B)/client/vorbis/floor1.o \
	  $(B)/client/vorbis/info.o \
	  $(B)/client/vorbis/lookup.o \
	  $(B)/client/vorbis/lpc.o \
	  $(B)/client/vorbis/lsp.o \
	  $(B)/client/vorbis/mapping0.o \
	  $(B)/client/vorbis/mdct.o \
	  $(B)/client/vorbis/misc.o \
	  $(B)/client/vorbis/psy.o \
	  $(B)/client/vorbis/registry.o \
	  $(B)/client/vorbis/res0.o \
	  $(B)/client/vorbis/sharedbook.o \
	  $(B)/client/vorbis/smallft.o \
	  $(B)/client/vorbis/synthesis.o \
	  $(B)/client/vorbis/vorbisenc.o \
	  $(B)/client/vorbis/vorbisfile.o \
	  $(B)/client/vorbis/window.o

# OPUS += \
		$(B)/client/opus/analysis.o \
		$(B)/client/opus/mlp.o \
		$(B)/client/opus/mlp_data.o \
		$(B)/client/opus/opus.o \
		$(B)/client/opus/opus_decoder.o \
		$(B)/client/opus/opus_encoder.o \
		$(B)/client/opus/opus_multistream.o \
		$(B)/client/opus/opus_multistream_encoder.o \
		$(B)/client/opus/opus_multistream_decoder.o \
		$(B)/client/opus/repacketizer.o \
		\
		$(B)/client/opus/bands.o \
		$(B)/client/opus/celt.o \
		$(B)/client/opus/cwrs.o \
		$(B)/client/opus/entcode.o \
		$(B)/client/opus/entdec.o \
		$(B)/client/opus/entenc.o \
		$(B)/client/opus/kiss_fft.o \
		$(B)/client/opus/laplace.o \
		$(B)/client/opus/mathops.o \
		$(B)/client/opus/mdct.o \
		$(B)/client/opus/modes.o \
		$(B)/client/opus/pitch.o \
		$(B)/client/opus/celt_encoder.o \
		$(B)/client/opus/celt_decoder.o \
		$(B)/client/opus/celt_lpc.o \
		$(B)/client/opus/quant_bands.o \
		$(B)/client/opus/rate.o \
		$(B)/client/opus/vq.o \
		\
		$(B)/client/opus/CNG.o \
		$(B)/client/opus/code_signs.o \
		$(B)/client/opus/init_decoder.o \
		$(B)/client/opus/decode_core.o \
		$(B)/client/opus/decode_frame.o \
		$(B)/client/opus/decode_parameters.o \
		$(B)/client/opus/decode_indices.o \
		$(B)/client/opus/decode_pulses.o \
		$(B)/client/opus/decoder_set_fs.o \
		$(B)/client/opus/dec_API.o \
		$(B)/client/opus/enc_API.o \
		$(B)/client/opus/encode_indices.o \
		$(B)/client/opus/encode_pulses.o \
		$(B)/client/opus/gain_quant.o \
		$(B)/client/opus/interpolate.o \
		$(B)/client/opus/LP_variable_cutoff.o \
		$(B)/client/opus/NLSF_decode.o \
		$(B)/client/opus/NSQ.o \
		$(B)/client/opus/NSQ_del_dec.o \
		$(B)/client/opus/PLC.o \
		$(B)/client/opus/shell_coder.o \
		$(B)/client/opus/tables_gain.o \
		$(B)/client/opus/tables_LTP.o \
		$(B)/client/opus/tables_NLSF_CB_NB_MB.o \
		$(B)/client/opus/tables_NLSF_CB_WB.o \
		$(B)/client/opus/tables_other.o \
		$(B)/client/opus/tables_pitch_lag.o \
		$(B)/client/opus/tables_pulses_per_block.o \
		$(B)/client/opus/VAD.o \
		$(B)/client/opus/control_audio_bandwidth.o \
		$(B)/client/opus/quant_LTP_gains.o \
		$(B)/client/opus/VQ_WMat_EC.o \
		$(B)/client/opus/HP_variable_cutoff.o \
		$(B)/client/opus/NLSF_encode.o \
		$(B)/client/opus/NLSF_VQ.o \
		$(B)/client/opus/NLSF_unpack.o \
		$(B)/client/opus/NLSF_del_dec_quant.o \
		$(B)/client/opus/process_NLSFs.o \
		$(B)/client/opus/stereo_LR_to_MS.o \
		$(B)/client/opus/stereo_MS_to_LR.o \
		$(B)/client/opus/check_control_input.o \
		$(B)/client/opus/control_SNR.o \
		$(B)/client/opus/init_encoder.o \
		$(B)/client/opus/control_codec.o \
		$(B)/client/opus/A2NLSF.o \
		$(B)/client/opus/ana_filt_bank_1.o \
		$(B)/client/opus/biquad_alt.o \
		$(B)/client/opus/bwexpander_32.o \
		$(B)/client/opus/bwexpander.o \
		$(B)/client/opus/debug.o \
		$(B)/client/opus/decode_pitch.o \
		$(B)/client/opus/inner_prod_aligned.o \
		$(B)/client/opus/lin2log.o \
		$(B)/client/opus/log2lin.o \
		$(B)/client/opus/LPC_analysis_filter.o \
		$(B)/client/opus/LPC_fit.o \
		$(B)/client/opus/LPC_inv_pred_gain.o \
		$(B)/client/opus/table_LSF_cos.o \
		$(B)/client/opus/NLSF2A.o \
		$(B)/client/opus/NLSF_stabilize.o \
		$(B)/client/opus/NLSF_VQ_weights_laroia.o \
		$(B)/client/opus/pitch_est_tables.o \
		$(B)/client/opus/resampler.o \
		$(B)/client/opus/resampler_down2_3.o \
		$(B)/client/opus/resampler_down2.o \
		$(B)/client/opus/resampler_private_AR2.o \
		$(B)/client/opus/resampler_private_down_FIR.o \
		$(B)/client/opus/resampler_private_IIR_FIR.o \
		$(B)/client/opus/resampler_private_up2_HQ.o \
		$(B)/client/opus/resampler_rom.o \
		$(B)/client/opus/sigm_Q15.o \
		$(B)/client/opus/sort.o \
		$(B)/client/opus/sum_sqr_shift.o \
		$(B)/client/opus/stereo_decode_pred.o \
		$(B)/client/opus/stereo_encode_pred.o \
		$(B)/client/opus/stereo_find_predictor.o \
		$(B)/client/opus/stereo_quant_pred.o \
		\
		$(B)/client/opus/apply_sine_window_FLP.o \
		$(B)/client/opus/corrMatrix_FLP.o \
		$(B)/client/opus/encode_frame_FLP.o \
		$(B)/client/opus/find_LPC_FLP.o \
		$(B)/client/opus/find_LTP_FLP.o \
		$(B)/client/opus/find_pitch_lags_FLP.o \
		$(B)/client/opus/find_pred_coefs_FLP.o \
		$(B)/client/opus/LPC_analysis_filter_FLP.o \
		$(B)/client/opus/LTP_analysis_filter_FLP.o \
		$(B)/client/opus/LTP_scale_ctrl_FLP.o \
		$(B)/client/opus/noise_shape_analysis_FLP.o \
		$(B)/client/opus/process_gains_FLP.o \
		$(B)/client/opus/regularize_correlations_FLP.o \
		$(B)/client/opus/residual_energy_FLP.o \
		$(B)/client/opus/warped_autocorrelation_FLP.o \
		$(B)/client/opus/wrappers_FLP.o \
		$(B)/client/opus/autocorrelation_FLP.o \
		$(B)/client/opus/burg_modified_FLP.o \
		$(B)/client/opus/bwexpander_FLP.o \
		$(B)/client/opus/energy_FLP.o \
		$(B)/client/opus/inner_product_FLP.o \
		$(B)/client/opus/k2a_FLP.o \
		$(B)/client/opus/LPC_inv_pred_gain_FLP.o \
		$(B)/client/opus/pitch_analysis_core_FLP.o \
		$(B)/client/opus/scale_copy_vector_FLP.o \
		$(B)/client/opus/scale_vector_FLP.o \
		$(B)/client/opus/schur_FLP.o \
		$(B)/client/opus/sort_FLP.o \
		\
	  $(B)/client/http.o \
	  $(B)/client/info.o \
	  $(B)/client/internal.o \
	  $(B)/client/opusfile.o \
	  $(B)/client/stream.o \
	  $(B)/client/wincerts.o



$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $^ $(LIBS) $(LDFLAGS) $(SDL_LIBS) -o $@

clean:
	@rm -rf $(BD)/client
	@rm -rf $(BR)/client

#############################################################################
# DEPENDENCIES
#############################################################################

ifdef B
D_FILES=$(shell find $(BD)/client -name '*.d')
endif

ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean makedirs release \
  targets tools toolsclean mkdirs \
	$(D_FILES)

.DEFAULT_GOAL := default