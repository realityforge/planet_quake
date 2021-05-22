MKFILE   := $(lastword $(MAKEFILE_LIST)) 

include make/configure.make
include make/platform.make

TARGET	    := libjpeg
SOURCES     := libs/jpeg-9d
INCLUDES    := 

#LIBS = -l

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
LIBOBJECTS := $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS)

Q3OBJ    := $(addprefix $(B)/libjpeg/,$(notdir $(LIBOBJECTS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS  := $(INCLUDE) -fsigned-char -MMD \
          -O2 -ftree-vectorize -g -ffast-math -fno-short-enums

define DO_JPEG_CC
	@echo "JPEG_CC $<"
	@$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

default:
	$(MAKE) -f $(MKFILE) B=$(BD) WORKDIR=libjpeg mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/libjpeg $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/libjpeg $(BR)/$(TARGET)$(SHLIBNAME)

ifdef B
$(B)/libjpeg/%.o: libs/jpeg-9d/%.c
	$(DO_JPEG_CC)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@
endif
