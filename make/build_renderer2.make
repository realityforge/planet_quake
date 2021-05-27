MKFILE         := $(lastword $(MAKEFILE_LIST)) 

include make/configure.make
BUILD_RENDERER_OPENGL2:=1
include make/platform.make

TARGET         := $(RENDERER_PREFIX)_opengl2_

SOURCES        := $(MOUNT_DIR)/renderer2 $(MOUNT_DIR)/renderer2/glsl $(MOUNT_DIR)/renderercommon
INCLUDES       := 

GLSLFFALLBACKS := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.glsl))
GLSLFILES      := $(addprefix glsl/,$(notdir $(GLSLFFALLBACKS)))
CFILES         := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
                  $(MOUNT_DIR)/qcommon/q_math.c $(MOUNT_DIR)/qcommon/q_shared.c \
                  $(MOUNT_DIR)/qcommon/puff.c
OBJS           := $(CFILES:.c=.o) 
Q3R2STRINGOBJ  := $(GLSLFILES:.glsl=.o)
Q3OBJ          := $(addprefix $(B)/rend2/,$(notdir $(OBJS))) \
                  $(addprefix $(B)/rend2/glsl/,$(notdir $(Q3R2STRINGOBJ)))
Q3R2STRCLEAN   := $(addsuffix _clean,$(addprefix $(B)/rend2/glsl/,$(notdir $(Q3R2STRINGOBJ))))

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
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=rend2 mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIR=rend2/glsl mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)$(SHLIBNAME)
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(TARGET)_clean

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=rend2 mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIR=rend2/glsl mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_CFLAGS)" $(BR)/$(TARGET)$(SHLIBNAME)
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(TARGET)_clean

clean:
	@rm -rf $(BD)/rend2 $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/rend2 $(BR)/$(TARGET)$(SHLIBNAME)
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) $(TARGET)_clean
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) $(TARGET)_clean

ifdef B
$(B)/rend2/%.o: code/qcommon/%.c
	$(DO_REND_CC)

$(B)/rend2/%.o: code/renderercommon/%.c
	$(DO_REND_CC)

$(B)/rend2/%.o: code/renderer2/%.c
	$(DO_REND_CC)

$(B)/rend2/glsl/%.c: code/renderer2/glsl/%.glsl
	$(DO_REF_STR)

$(B)/rend2/glsl/%.o_clean: code/renderer2/glsl/%.glsl
	@rm -f $@

$(B)/rend2/glsl/%.o: $(B)/rend2/glsl/%.c
	$(DO_REND_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(SHLIBCFLAGS) $(SHLIBLDFLAGS)
  
$(TARGET)_clean: $(Q3R2STRCLEAN)
endif
