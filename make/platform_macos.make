
BASE_CFLAGS += -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

BASE_CFLAGS += -Wno-unused-result

OPTIMIZE = -O2 -fvisibility=hidden

SHLIBEXT = dylib
SHLIBCFLAGS = -fPIC -fvisibility=hidden
SHLIBLDFLAGS = -dynamiclib $(LDFLAGS)

LDFLAGS =

ifneq ($(SDL_INCLUDE),)
  BASE_CFLAGS += $(SDL_INCLUDE)
  CLIENT_LDFLAGS = $(SDL_LIBS)
else
  BASE_CFLAGS += -I/Library/Frameworks/SDL2.framework/Headers
  CLIENT_LDFLAGS =  -F/Library/Frameworks -framework SDL2
endif

#  SERVER_LDFLAGS = -DUSE_MULTIVM_SERVER

LDFLAGS += -L$(MOUNT_DIR)/macosx -lxml2 -lpng
#BASE_CFLAGS += -L$(MOUNT_DIR)/macosx -I$(MOUNT_DIR)/RmlUi/Include
CLIENT_LDFLAGS += $(MOUNT_DIR)/macosx/libxml2.2.dylib $(MOUNT_DIR)/macosx/libpng.dylib
#  CLIENT_LDFLAGS += -lRmlCore -lxml2
#  CLIENT_LDFLAGS += 

DEBUG_CFLAGS = $(BASE_CFLAGS) -DDEBUG -D_DEBUG -g -O0
RELEASE_CFLAGS = $(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)
