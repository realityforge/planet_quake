MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := huffman

BUILD_HUFFMAN := 1
include make/platform.make

TARGET	      := huffman_$(SHLIBNAME)
SOURCES       := $(MOUNT_DIR)/qcommon
MUSL_SOURCE   := libs/musl-1.2.2
INCLUDES      := 
LIBS          := 
Q3OBJ         := $(B)/$(WORKDIR)/huffman.o $(B)/$(WORKDIR)/huffman_static.o $(B)/musl/memset.o $(B)/musl/memcpy.o

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS        := $(INCLUDE) -fsigned-char -MMD \
								 -DBUILD_HUFFMAN \
                 -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

define DO_HUFFMAN_CC
  $(echo_cmd) "HUFFMAN_CC $<"
  $(Q)$(CC) -o $@ $(SHLIBCFLAGS) $(CFLAGS) -c $<
endef

define DO_MUSL_CC
	$(echo_cmd) "MUSL_CC $<"
	$(Q)$(CC) -o $@ $(MUSL_INCLUDE) $(CFLAGS) $(MUSL_CFLAGS) -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIRS="$(WORKDIR) musl" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS="$(WORKDIR) musl" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf ./$(BD)/$(WORKDIR) ./$(BD)/$(TARGET)
	@rm -rf ./$(BR)/$(WORKDIR) ./$(BR)/$(TARGET)
	@rm -rf ./$(BD)/musl ./$(BR)/$(TARGET)
	@rm -rf ./$(BR)/musl ./$(BR)/$(TARGET)

ifdef B
$(B)/musl/%.o: $(MUSL_SOURCE)/src/string/%.c
	$(DO_MUSL_CC)

$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.c
	$(DO_HUFFMAN_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(LD) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS) 
endif
