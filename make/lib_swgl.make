MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := swgl

BUILD_LIBJPEG := 1
include make/platform.make

TARGET	      := opengl_$(SHLIBNAME)
SOURCES       := libs/swGL
INCLUDES      := libs/swGL
#LIBS = -l

GLCFILES      := Clipper.cpp \
						     CommandClearColor.cpp \
						     CommandClearDepth.cpp \
						     CommandDrawTriangle.cpp \
						     CommandPoisonPill.cpp \
						     CommandSynchronize.cpp \
						     Context.cpp \
						     DrawThread.cpp \
						     Main.cpp \
						     Log.cpp \
						     Matrix.cpp \
						     MatrixStack.cpp \
						     OpenGL.cpp \
						     Renderer.cpp \
						     DrawSurface.cpp \
						     TexCoordGen.cpp \
						     TextureManager.cpp \
						     TextureSampler.cpp \
						     Vector.cpp \
						     VertexPipeline.cpp \
						     Wiggle.cpp

# These objectfiles are included in libjpeg.a
LIBOBJECTS   := $(GLCFILES:.cpp=.o)
Q3OBJ        := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(LIBOBJECTS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS       := $(INCLUDE) -fsigned-char -MMD \
                -O2 -ftree-vectorize -g -ffast-math -fno-short-enums \
								-msse2 -lcstd++
CXXFLAGS      := $(CFLAGS) -std=c++17

define DO_SWGL_CXX
  @echo "SWGL_CXX $<"
	$(Q)$(CXX) -o $@ $(SHLIBCFLAGS) $(CXXFLAGS) -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf ./$(BD)/$(WORKDIR) ./$(BD)/$(TARGET)
	@rm -rf ./$(BR)/$(WORKDIR) ./$(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/swGL/%.cpp
	$(DO_SWGL_CXX)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS)
endif
