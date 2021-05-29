MKFILE         := $(lastword $(MAKEFILE_LIST))
WORKDIR        := rend2

BUILD_RENDERER_OPENGL2:=1
include make/platform.make

TARGET         := $(RENDERER_PREFIX)_opengl2_$(SHLIBNAME)
# TODO: change WORKDIR so both version can be build without overlapping
ifeq ($(USE_MULTIVM_CLIENT),1)
TARGET       := $(BOTLIB_PREFIX)_opengl2_mw_$(SHLIBNAME)
endif
SOURCES        := $(MOUNT_DIR)/renderer2 $(MOUNT_DIR)/renderer2/glsl $(MOUNT_DIR)/renderercommon
INCLUDES       := 

GLSLFFALLBACKS := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.glsl))
GLSLFILES      := $(addprefix glsl/,$(notdir $(GLSLFFALLBACKS)))
CFILES         := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
                  $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
OBJS           := $(CFILES:.c=.o) 
Q3R2STRINGOBJ  := $(GLSLFILES:.glsl=.o)
Q3OBJ          := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(OBJS))) \
                  $(addprefix $(B)/$(WORKDIR)/glsl/,$(notdir $(Q3R2STRINGOBJ)))
Q3R2STRCLEAN   := $(addsuffix _clean,$(addprefix $(B)/$(WORKDIR)/glsl/,$(notdir $(Q3R2STRINGOBJ))))

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS   := $(INCLUDE) -fsigned-char -ftree-vectorize \
            -ffast-math -fno-short-enums -MMD

define DO_REND_CC
  $(echo_cmd) "REND_CC $<"
  $(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

define DO_REF_STR
  $(echo_cmd) "REF_STR $<"
  $(Q)echo "const char *fallbackShader_$(notdir $(basename $<)) =" >> $@
  $(Q)cat $< | sed -e 's/^/\"/;s/$$/\\n\"/' | tr -d '\r' >> $@
  $(Q)echo ";" >> $@
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=$(WORKDIR)/glsl mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(TARGET)_clean

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=$(WORKDIR)/glsl mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(TARGET)_clean

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(TARGET)_clean
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(TARGET)_clean

ifdef B
$(B)/$(WORKDIR)/%.o: code/qcommon/%.c
	$(DO_REND_CC)

$(B)/$(WORKDIR)/%.o: code/renderercommon/%.c
	$(DO_REND_CC)

$(B)/$(WORKDIR)/%.o: code/renderer2/%.c
	$(DO_REND_CC)

$(B)/$(WORKDIR)/glsl/%.c: code/renderer2/glsl/%.glsl
	$(DO_REF_STR)

$(B)/$(WORKDIR)/glsl/%.o_clean: code/renderer2/glsl/%.glsl
	@rm -f $@

$(B)/$(WORKDIR)/glsl/%.o: $(B)/$(WORKDIR)/glsl/%.c
	$(DO_REND_CC)

$(B)/$(TARGET): $(Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
  
$(TARGET)_clean: $(Q3R2STRCLEAN)
endif
