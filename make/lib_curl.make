MKFILE        := $(lastword $(MAKEFILE_LIST))
WORKDIR       := libcurl
CURLDIR       := libs/curl-7.76.1

BUILD_LIBCURL := 1
include make/platform.make

TARGET	      := libcurl_$(SHLIBNAME)
SOURCES       := $(CURLDIR)/lib $(CURLDIR)/lib/vauth $(CURLDIR)/lib/vtls $(CURLDIR)/lib/vquic $(CURLDIR)/lib/vssh
INCLUDES      := $(CURLDIR)/lib $(CURLDIR)/include /usr/local/include
LIBS 				  := $(OPENSSL_LIBS) $(SSH_LIBS)

CURLFILES     := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CURLOBJS      := $(CURLFILES:.c=.o)
Q3OBJ         := $(addprefix $(B)/$(WORKDIR)/,$(notdir $(CURLOBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CFLAGS      := $(INCLUDE) -fsigned-char -O3 \
							 -isystem /opt/local/include \
							 -MMD \
							 $(OPENSSL_CFLAGS) $(SSH_CFLAGS) \
               -ftree-vectorize -ffast-math -fno-short-enums \
							 -DBUILDING_LIBCURL -DCURL_HIDDEN_SYMBOLS -DHAVE_CONFIG_H \
							 -Dlibcurl_EXPORTS

define DO_CURL_CC
  @echo "CURL_CC $<"
  @$(CC) $(SHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

debug:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BD) MAKEDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" $(BD)/$(TARGET)

release:
	$(echo_cmd) "MAKE $(TARGET)"
	@$(MAKE) -f $(MKFILE) B=$(BR) MAKEDIR=$(WORKDIR) mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" $(BR)/$(TARGET)

clean:
	@rm -rf $(BD)/$(WORKDIR) $(BD)/$(TARGET)
	@rm -rf $(BR)/$(WORKDIR) $(BR)/$(TARGET)

ifdef B
$(B)/$(WORKDIR)/%.o: $(CURLDIR)/lib/%.c
	$(DO_CURL_CC)

$(B)/$(WORKDIR)/%.o: $(CURLDIR)/lib/vauth/%.c
	$(DO_CURL_CC)

$(B)/$(WORKDIR)/%.o: $(CURLDIR)/lib/vtls/%.c
	$(DO_CURL_CC)

$(B)/$(WORKDIR)/%.o: $(CURLDIR)/lib/vquic/%.c
	$(DO_CURL_CC)

$(B)/$(WORKDIR)/%.o: $(CURLDIR)/lib/vssh/%.c
	$(DO_CURL_CC)

$(B)/$(TARGET): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -o $@ $(Q3OBJ) $(LIBS) $(SHLIBLDFLAGS)
endif
