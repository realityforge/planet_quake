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
Q3ASM          = $(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3ASM)
Q3LCC          = $(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3LCC)
Q3RCC          = $(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3RCC)
Q3CPP          = $(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3CPP)
LBURG          = $(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/lburg/$(TARGET_LBURG)
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
Q3LCC_BUILD   := $(BUILD_DIR)/release-$(COMPILE_PLATFORM)-$(COMPILE_ARCH)
WORKDIRS      += $(Q3LCC_WORKDIRS)
CLEANS        += $(Q3LCC_WORKDIRS) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3ASM) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3LCC) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3RCC) \
                 $(Q3LCC_WORKDIR)/$(TARGET_Q3CPP) \
                 $(Q3LCC_WORKDIR)/$(TARGET_LBURG)
Q3LCC_MKDIRS  := $(Q3LCC_BUILD).mkdirs $(addsuffix .mkdirs,$(addprefix $(Q3LCC_BUILD)/,$(Q3LCC_WORKDIRS)))

release: $(Q3LCC_MKDIRS)
	$(echo_cmd) "MAKE Q3LCC"
  ls -la /home/runner/work/planet_quake/planet_quake
  ls -la /home/runner/work/planet_quake/planet_quake/build
  ls -la /home/runner/work/planet_quake/planet_quake/build/release-linux-x86_64
  ls -la /home/runner/work/planet_quake/planet_quake/build/release-linux-x86_64/tools
	$(Q)$(MAKE) -f $(MKFILE) B=$(Q3LCC_BUILD) V=$(V) \
		WORKDIRS="$(Q3LCC_WORKDIRS)" \
    PLATFORM="$(COMPILE_PLATFORM)" \
    ARCH="$(COMPILE_ARCH)" \
		CFLAGS="$(TOOLS_CFLAGS) $(RELEASE_CFLAGS)" \
		LDFLAGS="$(TOOLS_LDFLAGS) $(RELEASE_LDFLAGS)" \
		$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3ASM) \
		$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3LCC) \
		$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3RCC) \
		$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/$(TARGET_Q3CPP) \
		$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/lburg/$(TARGET_LBURG)

clean:
	@rm -rf ./$(BD)/$(Q3LCC_WORKDIR) ./$(BD)/$(TARGET_Q3ASM) \
					./$(BD)/$(TARGET_Q3LCC)  ./$(BD)/$(TARGET_Q3RCC) \
					./$(BD)/$(TARGET_Q3CPP)  ./$(BD)/$(TARGET_LBURG)
	@rm -rf ./$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR) ./$(Q3LCC_BUILD)/$(TARGET_Q3ASM) \
					./$(Q3LCC_BUILD)/$(TARGET_Q3LCC)  ./$(Q3LCC_BUILD)/$(TARGET_Q3RCC) \
					./$(Q3LCC_BUILD)/$(TARGET_Q3CPP)  ./$(Q3LCC_BUILD)/$(TARGET_LBURG)
endif

Q3ASMOBJ       = $(Q3LCC_BUILD)/tools/asm/q3asm.o \
                 $(Q3LCC_BUILD)/tools/asm/q3vm.o \
                 $(Q3LCC_BUILD)/tools/asm/cmdlib.o

Q3LCCOBJ       = $(Q3LCC_BUILD)/tools/etc/lcc.o \
                 $(Q3LCC_BUILD)/tools/etc/bytecode.o

Q3CPPOBJ       = $(Q3LCC_BUILD)/tools/cpp/cpp.o \
                 $(Q3LCC_BUILD)/tools/cpp/lex.o \
                 $(Q3LCC_BUILD)/tools/cpp/nlist.o \
                 $(Q3LCC_BUILD)/tools/cpp/tokens.o \
                 $(Q3LCC_BUILD)/tools/cpp/macro.o \
                 $(Q3LCC_BUILD)/tools/cpp/eval.o \
                 $(Q3LCC_BUILD)/tools/cpp/include.o \
                 $(Q3LCC_BUILD)/tools/cpp/hideset.o \
                 $(Q3LCC_BUILD)/tools/cpp/getopt.o \
                 $(Q3LCC_BUILD)/tools/cpp/unix.o

Q3RCCOBJ       = $(Q3LCC_BUILD)/tools/rcc/alloc.o \
                 $(Q3LCC_BUILD)/tools/rcc/bind.o \
                 $(Q3LCC_BUILD)/tools/rcc/bytecode.o \
                 $(Q3LCC_BUILD)/tools/rcc/dag.o \
                 $(Q3LCC_BUILD)/tools/rcc/dagcheck.o \
                 $(Q3LCC_BUILD)/tools/rcc/decl.o \
                 $(Q3LCC_BUILD)/tools/rcc/enode.o \
                 $(Q3LCC_BUILD)/tools/rcc/error.o \
                 $(Q3LCC_BUILD)/tools/rcc/event.o \
                 $(Q3LCC_BUILD)/tools/rcc/expr.o \
                 $(Q3LCC_BUILD)/tools/rcc/gen.o \
                 $(Q3LCC_BUILD)/tools/rcc/init.o \
                 $(Q3LCC_BUILD)/tools/rcc/inits.o \
                 $(Q3LCC_BUILD)/tools/rcc/input.o \
                 $(Q3LCC_BUILD)/tools/rcc/lex.o \
                 $(Q3LCC_BUILD)/tools/rcc/list.o \
                 $(Q3LCC_BUILD)/tools/rcc/main.o \
                 $(Q3LCC_BUILD)/tools/rcc/null.o \
                 $(Q3LCC_BUILD)/tools/rcc/output.o \
                 $(Q3LCC_BUILD)/tools/rcc/prof.o \
                 $(Q3LCC_BUILD)/tools/rcc/profio.o \
                 $(Q3LCC_BUILD)/tools/rcc/simp.o \
                 $(Q3LCC_BUILD)/tools/rcc/stmt.o \
                 $(Q3LCC_BUILD)/tools/rcc/string.o \
                 $(Q3LCC_BUILD)/tools/rcc/sym.o \
                 $(Q3LCC_BUILD)/tools/rcc/symbolic.o \
                 $(Q3LCC_BUILD)/tools/rcc/trace.o \
                 $(Q3LCC_BUILD)/tools/rcc/tree.o \
                 $(Q3LCC_BUILD)/tools/rcc/types.o

LBURGOBJ       = $(Q3LCC_BUILD)/tools/lburg/lburg.o \
                 $(Q3LCC_BUILD)/tools/lburg/gram.o

DAGCHECK_C     = $(Q3LCC_BUILD)/tools/rcc/dagcheck.c

ifdef B

# tools

# override GNU Make built-in rule for converting gram.y to gram.c
%.c: %.y
	$(DO_YACC)

$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(DAGCHECK_C): $(LBURG) $(Q3LCCDIR)/src/dagcheck.md
	$(echo_cmd) "LBURG $(Q3LCCDIR)/src/dagcheck.md"
	$(Q)$(LBURG) $(Q3LCCDIR)/src/dagcheck.md $@

$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/rcc/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC)

$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/rcc/%.o: $(Q3LCCDIR)/src/%.c
	$(DO_TOOLS_CC)

$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/rcc/%.o: $(Q3LCCDIR)/etc/%.c
	$(DO_TOOLS_CC)

$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)
  
$(Q3LCC_BUILD)/$(Q3LCC_WORKDIR)/etc/%.o: $(Q3LCCDIR)/etc/%.c
	$(DO_TOOLS_CC)


# targets

$(Q3ASM): $(Q3LCC_MKDIRS) $(Q3ASMOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3ASMOBJ) $(TOOLS_LIBS)

$(Q3LCC): $(Q3LCC_MKDIRS) $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

$(Q3CPP): $(Q3LCC_MKDIRS) $(Q3CPPOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3CPPOBJ) $(TOOLS_LIBS)

$(LBURG): $(Q3LCC_MKDIRS) $(LBURGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(LBURGOBJ) $(TOOLS_LIBS)

$(Q3RCC): $(Q3LCC_MKDIRS) $(Q3RCCOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3RCCOBJ) $(TOOLS_LIBS)

endif
