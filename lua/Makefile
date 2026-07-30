# Makefile - lua/ subdirectory (now a thin wrapper)
#
# After the include/ + src/ + app/ + math/ + lua/ reorganization, all
# build orchestration lives at the top-level Makefile.  This file
# remains so that older scripts/READMEs that say "cd lua && make"
# continue to work, but the actual work is delegated to the parent.
#
# From here, `make` (any target) forwards to the top-level Makefile.
# See ../Makefile and ../docs/GKW_SCRIPTS_NOTE.md for the layout.

.PHONY: all build check clean data tables

# Forward every target to the parent Makefile.  This preserves the
# old "cd lua && make" workflow while keeping a single source of
# truth for the actual build rules.
%:
	@cd .. && $(MAKE) -f Makefile $@

all: build data check
