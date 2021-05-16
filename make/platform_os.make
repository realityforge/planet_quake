ifdef MINGW
include make/platform_win.make
else
ifeq ($(PLATFORM),darwin)
include make/platform_macos.make
else
ifeq ($(PLATFORM),js)
include make/platform_emjs.make
else
include make/platform_unix.make
endif
endif
endif
