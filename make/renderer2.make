MKFILE      := $(lastword $(MAKEFILE_LIST)) 

include make/platform.make
include make/configure.make

RENDERER_PREFIX  := $(CNAME)
TARGET	         := $(RENDERER_PREFIX)_opengl2_

SOURCES  := code/renderer2 code/renderer2/glsl code/renderercommon
INCLUDES := code/renderer2 code/renderer2/glsl code/renderercommon

#LIBS = -l
GLSLFFALLBACKS := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.glsl))
GLSLFILES      := $(addprefix glsl/,$(notdir $(GLSLFFALLBACKS)))
CFILES         := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
	                code/qcommon/q_math.c code/qcommon/q_shared.c \
							    code/qcommon/puff.c
OBJS          := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
Q3R2STRINGOBJ := $(GLSLFILES:.glsl=.o)
Q3OBJ         := $(addprefix $(B)/rend2/,$(notdir $(OBJS))) \
								 $(addprefix $(B)/rend2/glsl/,$(notdir $(Q3R2STRINGOBJ)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

PREFIX   = 
CC       = gcc
CXX      = g++
CFLAGS   = $(INCLUDE) -fsigned-char \
             -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
CXXFLAGS = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS  = $(CFLAGS)

SHLIBEXT     = dylib
SHLIBCFLAGS  = -fPIC -fno-common \
							 -DUSE_RENDERER_DLOPEN \
							 -DRENDERER_PREFIX=\\"$(RENDERER_PREFIX)\\"
SHLIBLDFLAGS = -dynamiclib $(LDFLAGS)
SHLIBNAME    = $(ARCH).$(SHLIBEXT)

define DO_REND_CC
	$(echo_cmd) "REND_CC $<"
	$(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) $(SDL_INCLUDE) -o $@ -c $<
endef

define DO_REF_STR
	$(echo_cmd) "REF_STR $<"
	$(Q)rm -f $@
	$(Q)echo "const char *fallbackShader_$(notdir $(basename $<)) =" >> $@
	$(Q)cat $< | sed -e 's/^/\"/;s/$$/\\n\"/' | tr -d '\r' >> $@
	$(Q)echo ";" >> $@
endef

mkdirs:
	@if [ ! -d $(B)/rend2 ];then $(MKDIR) $(B)/rend2;fi
	@if [ ! -d $(B)/rend2 ];then $(MKDIR) $(B)/rend2;fi

default:
	$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(TARGET)$(SHLIBNAME)

$(B)/rend2/%.o: code/qcommon/%.c
	$(DO_REND_CC)

$(B)/rend2/%.o: code/renderercommon/%.c
	$(DO_REND_CC)

$(B)/rend2/%.o: code/renderer2/%.c
	$(DO_REND_CC)

$(B)/rend2/glsl/%.c: code/renderer2/glsl/%.glsl
	$(DO_REF_STR)

$(B)/rend2/glsl/%.o: $(B)/renderer2/glsl/%.c
	$(DO_REND_CC)

$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $^ $(LIBS) $(SDL_LIBS) $(SHLIBLDFLAGS) -o $@

clean:
	@rm -rf $(B)/rend2
