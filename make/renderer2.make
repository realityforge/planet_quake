MKFILE      := $(lastword $(MAKEFILE_LIST)) 

#-include ./platform.make
-include ./configure0.make
MKDIR := mkdir

RENDERER_PREFIX  := $(CNAME)
TARGET	         := $(RENDERER_PREFIX)_opengl2_

SOURCES  := code/renderer2 code/renderercommon code/renderer2/glsl
INCLUDES := code/renderer2 code/renderercommon code/renderer2/glsl

#LIBS = -l

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c)) \
	            code/qcommon/q_math.c code/qcommon/q_shared.c \
							code/qcommon/puff.c
CPPFILES := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
Q3OBJ    := $(addprefix build/rend2/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

PREFIX   = 
CC       = gcc
CXX      = g++
CFLAGS   = $(INCLUDE) -fsigned-char \
             -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
CXXFLAGS = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS  = $(CFLAGS)

SHLIBEXT     = dylib
SHLIBCFLAGS  = -fPIC -fno-common -DUSE_RENDERER_DLOPEN
SHLIBLDFLAGS = -dynamiclib $(LDFLAGS)
SHLIBNAME    = $(ARCH).$(SHLIBEXT)

define DO_REND_CC
	@echo_cmd "REND_CC $<"
	$(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

define DO_REF_STR
	@echo_cmd "REF_STR $<"
	$(Q)rm -f $@
	$(Q)echo "const char *fallbackShader_$(notdir $(basename $<)) =" >> $@
	$(Q)cat $< | sed -e 's/^/\"/;s/$$/\\n\"/' | tr -d '\r' >> $@
	$(Q)echo ";" >> $@
endef

mkdirs:
	@echo "mkdir: $(MKDIR)"
	@if [ ! -d build/rend2 ];then $(MKDIR) build/rend2;fi

default:
	$(MAKE) -f $(MKFILE) mkdirs
	$(MAKE) -f $(MKFILE) "$(TARGET)$(SHLIBNAME)"

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
	@echo_cmd "LD $@"
	$(Q)$(CC) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@

clean:
	@rm -rf build/rend2
