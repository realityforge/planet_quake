MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := huffman

BUILD_HUFFMAN := 1
include make/platform.make

TARGET	      := huffman_$(SHLIBNAME)
SOURCES       := $(MOUNT_DIR)/wasm/lib
INCLUDES      := 
LIBS          := 
Q3OBJ         := $(B)/$(WORKDIR)/huffman.o $(B)/musl/memset.o $(B)/musl/memcpy.o

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS        := $(INCLUDE) -fsigned-char -MMD \
                 -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

define DO_HUFFMAN_CC
  $(echo_cmd) "HUFFMAN_CC $<"
  $(Q)$(CC) -o $@ $(SHLIBCFLAGS) $(CFLAGS) -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) WORKDIRS="$(WORKDIR) musl" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) WORKDIRS="$(WORKDIR) musl" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)
	@rm -rf $(BD)/musl $(BR)/$(TARGET)
	@rm -rf $(BR)/musl $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(SOURCES)/%.c
	$(DO_HUFFMAN_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(LD) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS) 
endif