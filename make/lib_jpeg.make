JPEG_WORKDIR       := libjpeg

BUILD_LIBJPEG := 1
ifneq ($(BUILD_CLIENT),1)
JPEG_MKFILE        := $(lastword $(MAKEFILE_LIST)) 
include make/platform.make
endif

JPEG_TARGET	       := libjpeg_$(SHLIBNAME)
JPEG_SOURCES       := libs/jpeg-9d
JPEG_INCLUDES      := 

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

JPEGOBJS     := $(addprefix $(B)/$(JPEG_WORKDIR)/,$(notdir $(LIBOBJECTS)))

export JPEG_INCLUDE	:= $(foreach dir,$(JPEG_INCLUDES),-I$(dir))

JPEG_CFLAGS       := $(JPEG_INCLUDE) -fsigned-char -MMD \
                -O2 -ftree-vectorize -g -ffast-math -fno-short-enums


define DO_JPEG_CC
  @echo "JPEG_CC $<"
  @$(CC) -o $@ $(SHLIBCFLAGS) $(JPEG_CFLAGS) -c $<
endef

ifneq ($(BUILD_CLIENT),1)

debug:
	$(echo_cmd) "MAKE $(JPEG_TARGET)"
	@$(MAKE) -f $(JPEG_MKFILE) B=$(BD) WORKDIRS=$(JPEG_WORKDIR) mkdirs
	@$(MAKE) -f $(JPEG_MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(JPEG_MKFILE) B=$(BD) CFLAGS="$(JPEG_CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(JPEG_TARGET)

release:
	$(echo_cmd) "MAKE $(JPEG_TARGET)"
	@$(MAKE) -f $(JPEG_MKFILE) B=$(BR) WORKDIRS=$(JPEG_WORKDIR) mkdirs
	@$(MAKE) -f $(JPEG_MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(JPEG_MKFILE) B=$(BR) CFLAGS="$(JPEG_CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(JPEG_TARGET)

clean:
	@rm -rf ./$(BD)/$(JPEG_WORKDIR) ./$(BD)/$(JPEG_TARGET)
	@rm -rf ./$(BR)/$(JPEG_WORKDIR) ./$(BR)/$(JPEG_TARGET)

else
WORKDIRS += $(JPEG_WORKDIR)
CLEANS 	 += $(JPEG_WORKDIR) $(JPEG_TARGET)
endif


ifdef B
$(B)/$(JPEG_WORKDIR)/%.o: $(JPEG_SOURCES)/%.c
	$(DO_JPEG_CC)

$(B)/$(JPEG_TARGET): $(JPEGOBJS) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(JPEGOBJS) $(SHLIBLDFLAGS) 
endif
