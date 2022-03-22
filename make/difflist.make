
DIFFLIST_DONE         := $(foreach vext,$(VALID_EXT), $(filter $(vext), $(DONE_STRIPPED)))
DIFFLIST_INVALID_EXT  := $(DIFFLIST_DONE)
DIFFLIST_INVALID_EXT  := $(foreach vext,$(VALID_EXT), $(foreach iext,$(CONVERT_EXT), $(patsubst $(vext),$(iext),$(DIFFLIST_INVALID_EXT))))
DIFFLIST_SOURCED      := $(foreach iext,$(CONVERT_EXT), $(filter $(iext), $(ALL_STRIPPED)))
DIFFLIST_VALID_EXT    := $(DIFFLIST_SOURCED)
DIFFLIST_VALID_EXT    := $(foreach vext,$(VALID_EXT), $(foreach iext,$(CONVERT_EXT), $(patsubst $(iext),$(vext),$(DIFFLIST_VALID_EXT))))
DIFFLIST_CONVERT      := $(filter-out $(DIFFLIST_INVALID_EXT),$(DIFFLIST_SOURCED))
DIFFLIST_INCLUDED     := $(filter $(DIFFLIST_VALID_EXT),$(DONE_STRIPPED))
