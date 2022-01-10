Q3LCC_WORKDIR := tools
Q3LCC_WORKDIRS:= $(Q3LCC_WORKDIR) $(Q3LCC_WORKDIR)/rcc \
                 $(Q3LCC_WORKDIR)/asm $(Q3LCC_WORKDIR)/etc \
                 $(Q3LCC_WORKDIR)/lburg $(Q3LCC_WORKDIR)/cpp

BUILD_Q3LCC   := 1
ifneq ($(MOD),1)
ifneq ($(BUILD_BASEQ3A),1)
NOT_INCLUDED_Q3LCC := 1
MKFILE        := $(lastword $(MAKEFILE_LIST)) 
include make/platform.make
endif
endif

Q3ASMDIR       = $(MOUNT_DIR)/../libs/tools/q3asm
LBURGDIR       = $(MOUNT_DIR)/../libs/tools/q3lcc/lburg
Q3CPPDIR       = $(MOUNT_DIR)/../libs/tools/q3lcc/cpp
Q3LCCDIR       = $(MOUNT_DIR)/../libs/tools/q3lcc

# targets
TARGET_Q3ASM   = q3asm
TARGET_Q3LCC   = q3lcc
TARGET_Q3RCC   = q3rcc
TARGET_Q3CPP   = q3cpp
TARGET_LBURG   = lburg

ifdef B
Q3ASM          = $(B)/$(Q3LCC_WORKDIR)/$(TARGET_Q3ASM)
Q3LCC          = $(B)/$(Q3LCC_WORKDIR)/$(TARGET_Q3LCC)
Q3RCC          = $(B)/$(Q3LCC_WORKDIR)/$(TARGET_Q3RCC)
Q3CPP          = $(B)/$(Q3LCC_WORKDIR)/$(TARGET_Q3CPP)
LBURG          = $(B)/$(Q3LCC_WORKDIR)/lburg/$(TARGET_LBURG)
endif

TOOLS_CFLAGS  := $(BASE_CFLAGS) \
                 -DTEMPDIR=\"$(TEMPDIR)\" -DSYSTEM=\"\" \
                 -I$(Q3LCCDIR)/src \
                 -I$(LBURGDIR)

TOOLS_LIBS    :=
TOOLS_LDFLAGS :=


define DO_TOOLS_CC
	$(echo_cmd) "TOOLS_CC $<"
	$(Q)$(CC) $(TOOLS_CFLAGS) -o $@ -c $<
endef

define DO_YACC
	$(echo_cmd) "YACC $<"
	$(Q)$(YACC) $<
	$(Q)mv -f y.tab.c $@
endef


ifdef NOT_INCLUDED_Q3LCC
debug:
	$(echo_cmd) "MAKE Q3LCC"
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) WORKDIRS="$(Q3LCC_WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BD) V=$(V) -j 8 \
	  WORKDIRS="$(Q3LCC_WORKDIRS)" \
	  CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)" \
	  LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" \
	  $(BD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3ASM) \
	  $(BD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3LCC) \
	  $(BD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3RCC) \
	  $(BD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3CPP) \
	  $(BD)/$(Q3LCC_WORKDIR)/lburg/$(TARGET_LBURG)

release:
	$(echo_cmd) "MAKE Q3LCC"
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) WORKDIRS="$(Q3LCC_WORKDIRS)" mkdirs
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) pre-build
	@$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) \
	  WORKDIRS="$(Q3LCC_WORKDIRS)" \
	  CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)" \
	  LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" \
	  $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3ASM) \
	  $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3LCC) \
	  $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3RCC) \
	  $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3CPP) \
	  $(BR)/$(Q3LCC_WORKDIR)/lburg/$(TARGET_LBURG)


clean:
	@rm -rf ./$(BD)/$(Q3LCC_WORKDIR) ./$(BD)/$(TARGET_Q3ASM) \
					./$(BD)/$(TARGET_Q3LCC)  ./$(BD)/$(TARGET_Q3RCC) \
					./$(BD)/$(TARGET_Q3CPP)  ./$(BD)/$(TARGET_LBURG)
	@rm -rf ./$(BR)/$(Q3LCC_WORKDIR) ./$(BR)/$(TARGET_Q3ASM) \
					./$(BR)/$(TARGET_Q3LCC)  ./$(BR)/$(TARGET_Q3RCC) \
					./$(BR)/$(TARGET_Q3CPP)  ./$(BR)/$(TARGET_LBURG)
else
WORKDIRS      += $(Q3LCC_WORKDIRS)
CLEANS        += $(Q3LCC_WORKDIRS) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3ASM) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3LCC) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3RCC) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3CPP) \
                 $(Q3LCC_WORKDIR)/$(TARGET_LBURG)
endif

Q3ASMOBJ       = $(B)/tools/asm/q3asm.o \
                 $(B)/tools/asm/q3vm.o \
                 $(B)/tools/asm/cmdlib.o

Q3LCCOBJ       = $(B)/tools/etc/lcc.o \
                 $(B)/tools/etc/bytecode.o

Q3CPPOBJ       = $(B)/tools/cpp/cpp.o \
                 $(B)/tools/cpp/lex.o \
                 $(B)/tools/cpp/nlist.o \
                 $(B)/tools/cpp/tokens.o \
                 $(B)/tools/cpp/macro.o \
                 $(B)/tools/cpp/eval.o \
                 $(B)/tools/cpp/include.o \
                 $(B)/tools/cpp/hideset.o \
                 $(B)/tools/cpp/getopt.o \
                 $(B)/tools/cpp/unix.o

Q3RCCOBJ       = $(B)/tools/rcc/alloc.o \
                 $(B)/tools/rcc/bind.o \
                 $(B)/tools/rcc/bytecode.o \
                 $(B)/tools/rcc/dag.o \
                 $(B)/tools/rcc/dagcheck.o \
                 $(B)/tools/rcc/decl.o \
                 $(B)/tools/rcc/enode.o \
                 $(B)/tools/rcc/error.o \
                 $(B)/tools/rcc/event.o \
                 $(B)/tools/rcc/expr.o \
                 $(B)/tools/rcc/gen.o \
                 $(B)/tools/rcc/init.o \
                 $(B)/tools/rcc/inits.o \
                 $(B)/tools/rcc/input.o \
                 $(B)/tools/rcc/lex.o \
                 $(B)/tools/rcc/list.o \
                 $(B)/tools/rcc/main.o \
                 $(B)/tools/rcc/null.o \
                 $(B)/tools/rcc/output.o \
                 $(B)/tools/rcc/prof.o \
                 $(B)/tools/rcc/profio.o \
                 $(B)/tools/rcc/simp.o \
                 $(B)/tools/rcc/stmt.o \
                 $(B)/tools/rcc/string.o \
                 $(B)/tools/rcc/sym.o \
                 $(B)/tools/rcc/symbolic.o \
                 $(B)/tools/rcc/trace.o \
                 $(B)/tools/rcc/tree.o \
                 $(B)/tools/rcc/types.o

LBURGOBJ       = $(B)/tools/lburg/lburg.o \
                 $(B)/tools/lburg/gram.o

DAGCHECK_C     = $(B)/tools/rcc/dagcheck.c

ifdef B

# tools

# override GNU Make built-in rule for converting gram.y to gram.c
%.c: %.y
	$(DO_YACC)

$(B)/$(Q3LCC_WORKDIR)/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(DAGCHECK_C): $(LBURG) $(Q3LCCDIR)/src/dagcheck.md
	$(echo_cmd) "LBURG $(Q3LCCDIR)/src/dagcheck.md"
	$(Q)$(LBURG) $(Q3LCCDIR)/src/dagcheck.md $@

$(B)/$(Q3LCC_WORKDIR)/rcc/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC)

$(B)/$(Q3LCC_WORKDIR)/rcc/%.o: $(Q3LCCDIR)/src/%.c
	$(DO_TOOLS_CC)

$(B)/$(Q3LCC_WORKDIR)/rcc/%.o: $(Q3LCCDIR)/etc/%.c
	$(DO_TOOLS_CC)

$(B)/$(Q3LCC_WORKDIR)/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(B)/$(Q3LCC_WORKDIR)/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)
  
$(B)/$(Q3LCC_WORKDIR)/etc/%.o: $(Q3LCCDIR)/etc/%.c
	$(DO_TOOLS_CC)


# targets

$(Q3ASM): $(Q3ASMOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(Q3LCC): $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

$(Q3CPP): $(Q3CPPOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(LBURG): $(LBURGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(Q3RCC): $(Q3RCCOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

endif
