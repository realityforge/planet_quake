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
Q3ASM          = $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3ASM)
Q3LCC          = $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3LCC)
Q3RCC          = $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3RCC)
Q3CPP          = $(BR)/$(Q3LCC_WORKDIR)/$(TARGET_Q3CPP)
LBURG          = $(BR)/$(Q3LCC_WORKDIR)/lburg/$(TARGET_LBURG)
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

release:
	$(echo_cmd) "MAKE Q3LCC"
	$(Q)$(MAKE) -f $(MKFILE) B=$(BR) V=$(V) \
	  WORKDIRS="$(Q3LCC_WORKDIRS)" \
	  CFLAGS="$(TOOLS_CFLAGS) $(RELEASE_CFLAGS)" \
	  LDFLAGS="$(TOOLS_LDFLAGS) $(RELEASE_LDFLAGS)" \
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

Q3ASMOBJ       = $(BR)/tools/asm/q3asm.o \
                 $(BR)/tools/asm/q3vm.o \
                 $(BR)/tools/asm/cmdlib.o

Q3LCCOBJ       = $(BR)/tools/etc/lcc.o \
                 $(BR)/tools/etc/bytecode.o

Q3CPPOBJ       = $(BR)/tools/cpp/cpp.o \
                 $(BR)/tools/cpp/lex.o \
                 $(BR)/tools/cpp/nlist.o \
                 $(BR)/tools/cpp/tokens.o \
                 $(BR)/tools/cpp/macro.o \
                 $(BR)/tools/cpp/eval.o \
                 $(BR)/tools/cpp/include.o \
                 $(BR)/tools/cpp/hideset.o \
                 $(BR)/tools/cpp/getopt.o \
                 $(BR)/tools/cpp/unix.o

Q3RCCOBJ       = $(BR)/tools/rcc/alloc.o \
                 $(BR)/tools/rcc/bind.o \
                 $(BR)/tools/rcc/bytecode.o \
                 $(BR)/tools/rcc/dag.o \
                 $(BR)/tools/rcc/dagcheck.o \
                 $(BR)/tools/rcc/decl.o \
                 $(BR)/tools/rcc/enode.o \
                 $(BR)/tools/rcc/error.o \
                 $(BR)/tools/rcc/event.o \
                 $(BR)/tools/rcc/expr.o \
                 $(BR)/tools/rcc/gen.o \
                 $(BR)/tools/rcc/init.o \
                 $(BR)/tools/rcc/inits.o \
                 $(BR)/tools/rcc/input.o \
                 $(BR)/tools/rcc/lex.o \
                 $(BR)/tools/rcc/list.o \
                 $(BR)/tools/rcc/main.o \
                 $(BR)/tools/rcc/null.o \
                 $(BR)/tools/rcc/output.o \
                 $(BR)/tools/rcc/prof.o \
                 $(BR)/tools/rcc/profio.o \
                 $(BR)/tools/rcc/simp.o \
                 $(BR)/tools/rcc/stmt.o \
                 $(BR)/tools/rcc/string.o \
                 $(BR)/tools/rcc/sym.o \
                 $(BR)/tools/rcc/symbolic.o \
                 $(BR)/tools/rcc/trace.o \
                 $(BR)/tools/rcc/tree.o \
                 $(BR)/tools/rcc/types.o

LBURGOBJ       = $(BR)/tools/lburg/lburg.o \
                 $(BR)/tools/lburg/gram.o

DAGCHECK_C     = $(BR)/tools/rcc/dagcheck.c

ifdef B

# tools

# override GNU Make built-in rule for converting gram.y to gram.c
%.c: %.y
	$(DO_YACC)

$(BR)/$(Q3LCC_WORKDIR)/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(DAGCHECK_C): $(LBURG) $(Q3LCCDIR)/src/dagcheck.md
	$(echo_cmd) "LBURG $(Q3LCCDIR)/src/dagcheck.md"
	$(Q)$(LBURG) $(Q3LCCDIR)/src/dagcheck.md $@

$(BR)/$(Q3LCC_WORKDIR)/rcc/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC)

$(BR)/$(Q3LCC_WORKDIR)/rcc/%.o: $(Q3LCCDIR)/src/%.c
	$(DO_TOOLS_CC)

$(BR)/$(Q3LCC_WORKDIR)/rcc/%.o: $(Q3LCCDIR)/etc/%.c
	$(DO_TOOLS_CC)

$(BR)/$(Q3LCC_WORKDIR)/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(BR)/$(Q3LCC_WORKDIR)/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)
  
$(BR)/$(Q3LCC_WORKDIR)/etc/%.o: $(Q3LCCDIR)/etc/%.c
	$(DO_TOOLS_CC)


# targets

mkdirs: $(BR)/ $(addsuffix .mkdirs,$(addprefix $(BR)/,$(Q3LCC_WORKDIRS)))
  @:

$(Q3ASM): mkdirs $(Q3ASMOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3ASMOBJ) $(TOOLS_LIBS)

$(Q3LCC): mkdirs $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

$(Q3CPP): mkdirs $(Q3CPPOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3CPPOBJ) $(TOOLS_LIBS)

$(LBURG): mkdirs $(LBURGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(LBURGOBJ) $(TOOLS_LIBS)

$(Q3RCC): mkdirs $(Q3RCCOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3RCCOBJ) $(TOOLS_LIBS)

endif
