REND_WORKDIR   := rend2
REND_SOURCE    := renderer2

BUILD_RENDERER_OPENGL2:=1
ifneq ($(BUILD_CLIENT),1)
MKFILE         := $(lastword $(MAKEFILE_LIST))
include make/platform.make
endif

REND_TARGET    := $(CNAME)_opengl2_$(SHLIBNAME)
ifeq ($(USE_MULTIVM_CLIENT),1)
REND_TARGET    := $(CNAME)_mw_opengl2_$(SHLIBNAME)
endif
REND_SOURCES   := $(MOUNT_DIR)/$(REND_SOURCE) $(MOUNT_DIR)/$(REND_SOURCE)/glsl \
									$(MOUNT_DIR)/renderercommon
GLSLFFALLBACKS := $(foreach dir,$(REND_SOURCES), $(wildcard $(dir)/*.glsl))
GLSLFILES      := $(addprefix glsl/,$(notdir $(GLSLFFALLBACKS)))
# TODO: switch this when other map formats are working
REND_CFILES    := $(foreach dir,$(REND_SOURCES), $(wildcard $(dir)/*.c))
ifeq ($(USE_RENDERER_DLOPEN),1)
REND_CFILES    += $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
endif
REND_OBJS      := $(REND_CFILES:.c=.o) 
Q3R2STRINGOBJ  := $(GLSLFILES:.glsl=.o)
REND_Q3OBJ     := $(addprefix $(B)/$(REND_WORKDIR)/,$(notdir $(REND_OBJS))) \
                  $(addprefix $(B)/$(REND_WORKDIR)/glsl/,$(notdir $(Q3R2STRINGOBJ)))
Q3R2STRCLEAN   := $(addsuffix _clean,$(addprefix $(B)/$(REND_WORKDIR)/glsl/,$(notdir $(Q3R2STRINGOBJ))))

REND_CFLAGS    ?= $(BASE_CFLAGS) $(INCLUDE)
ifeq ($(BUILD_CLIENT),1)
REND_CFLAGS    += $(CLIENT_CFLAGS)
endif
ifneq ($(BUILD_CLIENT),1)
REND_CFLAGS    += $(SHLIBCFLAGS)
endif

define DO_REND_CC
  $(echo_cmd) "REND_CC $<"
  $(Q)$(CC) -o $@ $(REND_CFLAGS) -c $<
endef

define DO_REF_STR
  $(echo_cmd) "REF_STR $<"
  $(Q)echo "const char *fallbackShader_$(notdir $(basename $<)) =" >> $@
  $(Q)cat $< | sed -e 's/^/\"/;s/$$/\\n\"/' | tr -d '\r' >> $@
  $(Q)echo ";" >> $@
endef

ifneq ($(BUILD_CLIENT),1)
debug:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS="$(REND_WORKDIR) $(REND_WORKDIR)/glsl" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) REND_CFLAGS="$(REND_CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(REND_TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(REND_TARGET)_clean

release:
	$(echo_cmd) "MAKE $(REND_TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(REND_WORKDIR) $(REND_WORKDIR)/glsl" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) REND_CFLAGS="$(REND_CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(REND_TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(REND_TARGET)_clean

clean:
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(REND_TARGET)_clean
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(REND_TARGET)_clean
	@rm -rf ./$(BD)/$(REND_WORKDIR) ./$(BD)/$(REND_TARGET)
	@rm -rf ./$(BR)/$(REND_WORKDIR) ./$(BR)/$(REND_TARGET)
else
WORKDIRS += $(REND_WORKDIR) $(REND_WORKDIR)/glsl
CLEANS 	 += $(REND_WORKDIR) $(REND_TARGET)
endif

ifdef B
$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/qcommon/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/renderercommon/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/%.o: $(MOUNT_DIR)/$(REND_SOURCE)/%.c
	$(DO_REND_CC)

$(B)/$(REND_WORKDIR)/glsl/%.c: $(MOUNT_DIR)/$(REND_SOURCE)/glsl/%.glsl
	$(DO_REF_STR)

$(B)/$(REND_WORKDIR)/glsl/%.o_clean: $(MOUNT_DIR)/$(REND_SOURCE)/glsl/%.glsl
	@rm -f ./$@

$(B)/$(REND_WORKDIR)/glsl/%.o: $(B)/$(REND_WORKDIR)/glsl/%.c
	$(DO_REND_CC)

$(B)/$(REND_TARGET): $(REND_Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(REND_Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
  
$(REND_TARGET)_clean: $(Q3R2STRCLEAN)
endif
