MKFILE        := $(lastword $(MAKEFILE_LIST)) 
WORKDIR       := libjpeg

BUILD_LIBJPEG := 1
include make/platform.make

TARGET	      := libjpeg_$(SHLIBNAME)
SOURCES       := libs/jpeg-9d
INCLUDES      := 
LIBS          :=

COMOBJECTS   := jaricom.o jcomapi.o jutils.o jerror.o jmemmgr.o jmemnobs.o $(SYSDEPMEM)
# compression library object files
CLIBOBJECTS  := jcapimin.o jcapistd.o jcarith.o jctrans.o jcparam.o \
                jdatadst.o jcinit.o jcmaster.o jcmarker.o jcmainct.o jcprepct.o \
                jccoefct.o jccolor.o jcsample.o jchuff.o jcdctmgr.o jfdctfst.o \
                jfdctflt.o jfdctint.o
# decompression library object files
DLIBOBJECTS  := jdapimin.o jdapistd.o jdarith.o jdtrans.o jdatasrc.o \
                jdmaster.o jdinput.o jdmarker.o jdhuff.o jdmainct.o \
                jdcoefct.o jdpostct.o jddctmgr.o jidctfst.o jidctflt.o \
                jidctint.o jdsample.o jdcolor.o jquant1.o jquant2.o jdmerge.o
# These objectfiles are included in libjpeg.a
LIBOBJECTS   := $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS)

Q3OBJ        := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(LIBOBJECTS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS       := $(INCLUDE) -fsigned-char -MMD \
                -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

define DO_JPEG_CC
  @echo "JPEG_CC $<"
  @$(CC) -o $@ $(SHLIBCFLAGS) $(CFLAGS) -c $<
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
$(B)/$(WORKDIR)/%.o: libs/jpeg-9d/%.c
	$(DO_JPEG_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS) 
endif
